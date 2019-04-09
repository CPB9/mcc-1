#include "../traits/Trait.h"
#include "../traits/TraitRadioCalibration.h"
#include "mcc/net/NetLoggerInf.h"
#include "mcc/msg/FwdExt.h"

#include <bmcl/Logging.h>

#include <array>

#include <decode/ast/Ast.h>
#include <decode/ast/Component.h>
#include <decode/ast/Function.h>
#include <decode/core/Diagnostics.h>
#include <decode/parser/Package.h>

#include <photon/core/Rc.h>
#include <photon/groundcontrol/Atoms.h>

#include <photon/groundcontrol/TmParamUpdate.h>
#include <photon/groundcontrol/AllowUnsafeMessageType.h>
#include <photon/groundcontrol/GcStructs.h>
#include <photon/groundcontrol/GcCmd.h>
#include <photon/groundcontrol/TmState.h>
#include <photon/model/NodeViewUpdater.h>
#include <photon/model/CoderState.h>
#include <photon/model/CmdNode.h>
#include <Photon.hpp>

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NotificationPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(photongen::fcu::RcChannels);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::CmdCalibrationStart);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(bmcl::SharedBytes);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(::photon::ProjectUpdate::ConstPointer);

namespace mccphoton {

constexpr const std::chrono::milliseconds _stickDetectSettle = std::chrono::milliseconds(500);

static const char* _imageHome("radioHome.png");
static const char* _imageThrottleUp("radioThrottleUp.png");
static const char* _imageThrottleDown("radioThrottleDown.png");
static const char* _imageYawLeft("radioYawLeft.png");
static const char* _imageYawRight("radioYawRight.png");
static const char* _imageRollLeft("radioRollLeft.png");
static const char* _imageRollRight("radioRollRight.png");
static const char* _imagePitchUp("radioPitchUp.png");
static const char* _imagePitchDown("radioPitchDown.png");
static const char* _imageSwitchMinMax("radioSwitchMinMax.png");

const struct TraitRadioCalibration::FunctionInfo TraitRadioCalibration::_rgFunctionInfoPX4[TraitRadioCalibration::rcCalFunctionMax] = {
    { "RC_MAP_ROLL" },
    { "RC_MAP_PITCH" },
    { "RC_MAP_YAW" },
    { "RC_MAP_THROTTLE" }
};

TraitRadioCalibration::TraitRadioCalibration(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const mccmsg::ProtocolId& id, const std::string& name)
    : caf::event_based_actor(cfg), _helper(id, core, this), _id(id), _activated(false), _name(name), _core{ core }, _broker{ broker }
    , _currentStep(-1)
    , _chanCount(0)
    , _rcCalState(rcCalStateChannelWait)
    , _started(false)
{
    _resetInternalCalibrationValues();
    _calibrationStatus._sensor = mccmsg::CalibrationSensor::Radio;
}

void TraitRadioCalibration::processChannelsMessage(photongen::fcu::RcChannels& channels)
{
    int pwmValues[_chanMaxPX4];

    for (int i = 0; i < _chanMaxPX4; i++) {
        uint16_t channelValue = channels.chanRaw()[i];

        if (i < channels.chanCount()) {
            pwmValues[i] = channelValue == UINT16_MAX ? -1 : channelValue;
        }
        else {
            pwmValues[i] = -1;
        }
    }

    /////////////////////////////////////////////
    int maxChannel = channels.chanCount();


    for (int channel = 0; channel < maxChannel; channel++) {
        int channelValue = pwmValues[channel];

        if (channelValue != -1) {
            //BMCL_DEBUG() << "Raw value" << channel << channelValue;

            _rcRawValue[channel] = channelValue;
            //emit channelRCValueChanged(channel, channelValue);

            // Signal attitude rc values to Qml if mapped
            if (_rgChannelInfo[channel].function != rcCalFunctionMax) {
                switch (_rgChannelInfo[channel].function) {
                case rcCalFunctionRoll:
                    //emit rollChannelRCValueChanged(channelValue);
                    break;
                case rcCalFunctionPitch:
                    //emit pitchChannelRCValueChanged(channelValue);
                    break;
                case rcCalFunctionYaw:
                    //emit yawChannelRCValueChanged(channelValue);
                    break;
                case rcCalFunctionThrottle:
                    //emit throttleChannelRCValueChanged(channelValue);
                    break;
                default:
                    break;

                }
            }

            if (_currentStep == -1) {
                return;
                //                if (_chanCount != channel.chancount) {
                //                    _chanCount = channelCount;
                //                    emit channelCountChanged(_chanCount);
                //                }
            }
            else {
                const stateMachineEntry* state = _getStateMachineEntry(_currentStep);
                Q_ASSERT(state);
                if (state->rcInputFn) {
                    (this->*state->rcInputFn)(state->function, channel, channelValue);
                }
            }
        }
    }
}

const TraitRadioCalibration::stateMachineEntry* TraitRadioCalibration::_getStateMachineEntry(int step) const
{
    static const char* msgBeginPX4 = "Lower the Throttle stick all the way down as shown in diagram.\n\n"
        "It is recommended to disconnect all motors for additional safety, however, the system is designed to not arm during the calibration.\n\n"
        "Click Next to continue";
    static const char* msgThrottleUp = "Move the Throttle stick all the way up and hold it there...";
    static const char* msgThrottleDown = "Move the Throttle stick all the way down and leave it there...";
    static const char* msgYawLeft = "Move the Yaw stick all the way to the left and hold it there...";
    static const char* msgYawRight = "Move the Yaw stick all the way to the right and hold it there...";
    static const char* msgRollLeft = "Move the Roll stick all the way to the left and hold it there...";
    static const char* msgRollRight = "Move the Roll stick all the way to the right and hold it there...";
    static const char* msgPitchDown = "Move the Pitch stick all the way down and hold it there...";
    static const char* msgPitchUp = "Move the Pitch stick all the way up and hold it there...";
    static const char* msgPitchCenter = "Allow the Pitch stick to move back to center...";
    static const char* msgSwitchMinMax = "Move all the transmitter switches and/or dials back and forth to their extreme positions.";
    static const char* msgComplete = "All settings have been captured. Click Next to write the new parameters to your board.";

    static const stateMachineEntry rgStateMachinePX4[] = {
        //Function
        { rcCalFunctionMax,                 msgBeginPX4,        _imageHome,         &TraitRadioCalibration::_inputCenterWaitBegin,   &TraitRadioCalibration::_saveAllTrims,       NULL },
        { rcCalFunctionThrottle,            msgThrottleUp,      _imageThrottleUp,   &TraitRadioCalibration::_inputStickDetect,       NULL,                                           NULL },
        { rcCalFunctionThrottle,            msgThrottleDown,    _imageThrottleDown, &TraitRadioCalibration::_inputStickMin,          NULL,                                           NULL },
        { rcCalFunctionYaw,                 msgYawRight,        _imageYawRight,     &TraitRadioCalibration::_inputStickDetect,       NULL,                                           NULL },
        { rcCalFunctionYaw,                 msgYawLeft,         _imageYawLeft,      &TraitRadioCalibration::_inputStickMin,          NULL,                                           NULL },
        { rcCalFunctionRoll,                msgRollRight,       _imageRollRight,    &TraitRadioCalibration::_inputStickDetect,       NULL,                                           NULL },
        { rcCalFunctionRoll,                msgRollLeft,        _imageRollLeft,     &TraitRadioCalibration::_inputStickMin,          NULL,                                           NULL },
        { rcCalFunctionPitch,               msgPitchUp,         _imagePitchUp,      &TraitRadioCalibration::_inputStickDetect,       NULL,                                           NULL },
        { rcCalFunctionPitch,               msgPitchDown,       _imagePitchDown,    &TraitRadioCalibration::_inputStickMin,          NULL,                                           NULL },
        { rcCalFunctionPitch,               msgPitchCenter,     _imageHome,         &TraitRadioCalibration::_inputCenterWait,        NULL,                                           NULL },
        { rcCalFunctionMax,                 msgSwitchMinMax,    _imageSwitchMinMax, &TraitRadioCalibration::_inputSwitchMinMax,      &TraitRadioCalibration::_advanceState,       NULL },
        { rcCalFunctionMax,                 msgComplete,        _imageThrottleDown, NULL,                                       &TraitRadioCalibration::_writeCalibration,   NULL },
    };

    bool badStep = false;
    if (step < 0) {
        badStep = true;
    }
    if (true) {
        if (step >= (int)(sizeof(rgStateMachinePX4) / sizeof(rgStateMachinePX4[0]))) {
            badStep = true;
        }
    }
    else {
        step = 0;
    }
    if (badStep) {
        BMCL_WARNING() << "Bad step value" << step;
        step = 0;
    }

    const stateMachineEntry* stateMachine = rgStateMachinePX4;

    return &stateMachine[step];
}

const TraitRadioCalibration::FunctionInfo* TraitRadioCalibration::_functionInfo(void) const
{
    return _rgFunctionInfoPX4;
}

void TraitRadioCalibration::_advanceState(void)
{
    _currentStep++;
    _setupCurrentState();
}

void TraitRadioCalibration::_setupCurrentState(void)
{
    const stateMachineEntry* state = _getStateMachineEntry(_currentStep);

    auto instructions = state->instructions;
    auto helpImage = state->image;

    BMCL_DEBUG() << "Radio calibration: " << instructions;

    _calibrationStatus.message = instructions;
    _setHelpImage(helpImage);

    _stickDetectChannel = _chanMaxPX4;
    _stickDetectSettleStarted = false;

    _rcCalSaveCurrentValues();

    _calibrationStatus.nextEnabled = (state->nextFn != NULL);
    _calibrationStatus.skipEnabled = (state->skipFn != NULL);

    sendTm();
}

void TraitRadioCalibration::_inputCenterWaitBegin(enum rcCalFunctions function, int channel, int value)
{
    Q_UNUSED(function);
    Q_UNUSED(channel);
    Q_UNUSED(value);

    // FIXME: Doesn't wait for center
    _calibrationStatus.nextEnabled = true;
    sendTm();
}

void TraitRadioCalibration::_inputStickDetect(enum rcCalFunctions function, int channel, int value)
{
    //BMCL_DEBUG() << "_inputStickDetect function:channel:value" << _functionInfo()[function].parameterName << channel << value;

    // If this channel is already used in a mapping we can't use it again
    if (_rgChannelInfo[channel].function != rcCalFunctionMax) {
        return;
    }

    if (_stickDetectChannel == _chanMaxPX4) {
        // We have not detected enough movement on a channel yet

        if (abs(_rcValueSave[channel] - value) > _rcCalMoveDelta) {
            // Stick has moved far enough to consider it as being selected for the function

            BMCL_DEBUG() << "_inputStickDetect starting settle wait, function:channel:value" << function << channel << value;

            // Setup up to detect stick being pegged to min or max value
            _stickDetectChannel = channel;
            _stickDetectInitialValue = value;
            _stickDetectValue = value;
        }
    }
    else if (channel == _stickDetectChannel) {
        if (_stickSettleComplete(value)) {
            ChannelInfo* info = &_rgChannelInfo[channel];

            // Stick detection is complete. Stick should be at max position.
            // Map the channel to the function
            _rgFunctionChannelMapping[function] = channel;
            info->function = function;

            // Channel should be at max value, if it is below initial set point the the channel is reversed.
            info->reversed = value < _rcValueSave[channel];

            BMCL_DEBUG() << "_inputStickDetect settle complete, function:channel:value:reversed" << function << channel << value << info->reversed;

            if (info->reversed) {
                _rgChannelInfo[channel].rcMin = value;
            }
            else {
                _rgChannelInfo[channel].rcMax = value;
            }

            //_signalAllAttiudeValueChanges();

            _advanceState();
        }
    }
}

void TraitRadioCalibration::_inputStickMin(enum rcCalFunctions function, int channel, int value)
{
    // We only care about the channel mapped to the function we are working on
    if (_rgFunctionChannelMapping[function] != channel) {
        return;
    }

    if (_stickDetectChannel == _chanMaxPX4) {
        // Setup up to detect stick being pegged to extreme position
        if (_rgChannelInfo[channel].reversed) {
            if (value > _rcCalPWMCenterPoint + _rcCalMoveDelta) {
                _stickDetectChannel = channel;
                _stickDetectInitialValue = value;
                _stickDetectValue = value;
            }
        }
        else {
            if (value < _rcCalPWMCenterPoint - _rcCalMoveDelta) {
                _stickDetectChannel = channel;
                _stickDetectInitialValue = value;
                _stickDetectValue = value;
            }
        }
    }
    else {
        // We are waiting for the selected channel to settle out

        if (_stickSettleComplete(value)) {
            ChannelInfo* info = &_rgChannelInfo[channel];

            // Stick detection is complete. Stick should be at min position.
            if (info->reversed) {
                _rgChannelInfo[channel].rcMax = value;
            }
            else {
                _rgChannelInfo[channel].rcMin = value;
            }

            // Check if this is throttle and set trim accordingly
            if (function == rcCalFunctionThrottle) {
                _rgChannelInfo[channel].rcTrim = value;
            }
            // XXX to support configs which can reverse they need to check a reverse
            // flag here and not do this.

            _advanceState();
        }
    }
}

void TraitRadioCalibration::_inputCenterWait(enum rcCalFunctions function, int channel, int value)
{
    // We only care about the channel mapped to the function we are working on
    if (_rgFunctionChannelMapping[function] != channel) {
        return;
    }

    if (_stickDetectChannel == _chanMaxPX4) {
        // Sticks have not yet moved close enough to center

        if (abs(_rcCalPWMCenterPoint - value) < _rcCalRoughCenterDelta) {
            // Stick has moved close enough to center that we can start waiting for it to settle
            _stickDetectChannel = channel;
            _stickDetectInitialValue = value;
            _stickDetectValue = value;
        }
    }
    else {
        if (_stickSettleComplete(value)) {
            _advanceState();
        }
    }
}

void TraitRadioCalibration::_inputSwitchMinMax(enum rcCalFunctions function, int channel, int value)
{
    Q_UNUSED(function);

    // If the channel is mapped we already have min/max
    if (_rgChannelInfo[channel].function != rcCalFunctionMax) {
        return;
    }

    if (abs(_rcCalPWMCenterPoint - value) > _rcCalMoveDelta) {
        // Stick has moved far enough from center to consider for min/max
        if (value < _rcCalPWMCenterPoint) {
            int minValue = qMin(_rgChannelInfo[channel].rcMin, value);

            BMCL_DEBUG() << "_inputSwitchMinMax setting min channel:min" << channel << minValue;

            _rgChannelInfo[channel].rcMin = minValue;
        }
        else {
            int maxValue = qMax(_rgChannelInfo[channel].rcMax, value);

            BMCL_DEBUG() << "_inputSwitchMinMax setting max channel:max" << channel << maxValue;

            _rgChannelInfo[channel].rcMax = maxValue;
        }
    }
}

void TraitRadioCalibration::_inputSwitchDetect(enum rcCalFunctions function, int channel, int value)
{
    _switchDetect(function, channel, value, true /* move to next step after detection */);
}

void TraitRadioCalibration::_switchDetect(enum rcCalFunctions function, int channel, int value, bool moveToNextStep)
{
    // If this channel is already used in a mapping we can't use it again
    if (_rgChannelInfo[channel].function != rcCalFunctionMax) {
        return;
    }

    if (abs(_rcValueSave[channel] - value) > _rcCalMoveDelta) {
        ChannelInfo* info = &_rgChannelInfo[channel];

        // Switch has moved far enough to consider it as being selected for the function

        // Map the channel to the function
        _rgChannelInfo[channel].function = function;
        _rgFunctionChannelMapping[function] = channel;
        info->function = function;

        BMCL_DEBUG() << "Function:" << function << "mapped to:" << channel;

        if (moveToNextStep) {
            _advanceState();
        }
    }
}

void TraitRadioCalibration::_saveAllTrims(void)
{
    for (int i = 0; i < _chanCount; i++) {
        BMCL_DEBUG() << "_saveAllTrims channel trim" << i << _rcRawValue[i];
        _rgChannelInfo[i].rcTrim = _rcRawValue[i];
    }
    _advanceState();
}

bool TraitRadioCalibration::_stickSettleComplete(int value)
{
    // We are waiting for the stick to settle out to a max position

    if (abs(_stickDetectValue - value) > _rcCalSettleDelta) {
        // Stick is moving too much to consider stopped

        BMCL_DEBUG() << "_stickSettleComplete still moving, _stickDetectValue:value" << _stickDetectValue << value;

        _stickDetectValue = value;
        _stickDetectSettleStarted = false;
    }
    else {
        // Stick is still positioned within the specified small range

        if (_stickDetectSettleStarted) {
            // We have already started waiting

            if (_stickDetectSettleElapsed.passed() > _stickDetectSettle) {
                // Stick has stayed positioned in one place long enough, detection is complete.
                return true;
            }
        }
        else {
            // Start waiting for the stick to stay settled for _stickDetectSettleWaitMSecs msecs

            BMCL_DEBUG() << "_stickSettleComplete starting settle timer, _stickDetectValue:value" << _stickDetectValue << value;

            _stickDetectSettleStarted = true;
            _stickDetectSettleElapsed.start();
        }
    }

    return false;
}

void TraitRadioCalibration::_validateCalibration(void)
{
    for (int chan = 0; chan < _chanMaxPX4; chan++) {
        struct ChannelInfo* info = &_rgChannelInfo[chan];

        if (chan < _chanMaxPX4) {
            // Validate Min/Max values. Although the channel appears as available we still may
            // not have good min/max/trim values for it. Set to defaults if needed.
            if (info->rcMin > _rcCalPWMValidMinValue || info->rcMax < _rcCalPWMValidMaxValue) {
                BMCL_DEBUG() << "_validateCalibration resetting channel" << chan;
                info->rcMin = _rcCalPWMDefaultMinValue;
                info->rcMax = _rcCalPWMDefaultMaxValue;
                info->rcTrim = info->rcMin + ((info->rcMax - info->rcMin) / 2);
            }
            else {
                switch (_rgChannelInfo[chan].function)
                {
                case rcCalFunctionThrottle:
                case rcCalFunctionYaw:
                case rcCalFunctionRoll:
                case rcCalFunctionPitch:
                    // Make sure trim is within min/max
                    if (info->rcTrim < info->rcMin) {
                        info->rcTrim = info->rcMin;
                    }
                    else if (info->rcTrim > info->rcMax) {
                        info->rcTrim = info->rcMax;
                    }
                    break;
                default:
                    // Non-attitude control channels have calculated trim
                    info->rcTrim = info->rcMin + ((info->rcMax - info->rcMin) / 2);
                    break;
                }

            }
        }
        else {
            // Unavailable channels are set to defaults
            BMCL_DEBUG() << "_validateCalibration resetting unavailable channel" << chan;
            info->rcMin = _rcCalPWMDefaultMinValue;
            info->rcMax = _rcCalPWMDefaultMaxValue;
            info->rcTrim = info->rcMin + ((info->rcMax - info->rcMin) / 2);
            info->reversed = false;
        }
    }
}

void TraitRadioCalibration::_writeCalibration(void)
{
    _validateCalibration();

    std::array<photongen::fcu::RcRatio, 18> rcRatios;
    for (int i = 0; i < 18; i++)
    {
        auto info = &_rgChannelInfo[i];
        rcRatios[i].setTrim(info->rcTrim);
        rcRatios[i].setRcMin(info->rcMin);
        rcRatios[i].setRcMax(info->rcMax);
        rcRatios[i].setRev(info->reversed);
    }

    uint8_t rollChannel     = 0;
    uint8_t pitchChannel    = 0;
    uint8_t yawChannel      = 0;
    uint8_t throttleChannel = 0;

    if(_rgFunctionChannelMapping[rcCalFunctionRoll] != _chanMaxPX4)
        rollChannel     = _rgFunctionChannelMapping[rcCalFunctionRoll] + 1;
    if (_rgFunctionChannelMapping[rcCalFunctionPitch] != _chanMaxPX4)
        pitchChannel    = _rgFunctionChannelMapping[rcCalFunctionPitch] + 1;
    if (_rgFunctionChannelMapping[rcCalFunctionYaw] != _chanMaxPX4)
        yawChannel      = _rgFunctionChannelMapping[rcCalFunctionYaw] + 1;
    if (_rgFunctionChannelMapping[rcCalFunctionThrottle] != _chanMaxPX4)
        throttleChannel = _rgFunctionChannelMapping[rcCalFunctionThrottle] + 1;

    // Write function mapping parameters

    bmcl::Buffer dest;
    ::photon::CoderState state(::photon::OnboardTime::now());
    if (!_prj->interface()->encodeCmdFcuWriteRadioCalibrationRatios(rcRatios, rollChannel, pitchChannel, yawChannel, throttleChannel, &dest, &state))
    {
        BMCL_CRITICAL() << "Can't encodeCmdFcuWriteRadioCalibrationRatios!";
        return;
    }

    bmcl::Bytes pkt = dest.asBytes();
    send(_broker, mccnet::send_cmd_atom::value, bmcl::SharedBytes::create(pkt.data(), pkt.size()));

    _calibrationStatus.progress = 100;
    _calibrationStatus.done = true;
    _started = false;

    sendTm();
}

caf::behavior TraitRadioCalibration::make_behavior()
{
    return
    {
         [this](activated_atom)
         {
             if (!_activated)
             {
                 _activated = true;
             }
         }
       , [this](deactivated_atom)
           {
               if (_activated)
               {
                   _activated = false;
               }
           }
       , [this](::photon::SetProjectAtom, const ::photon::ProjectUpdate::ConstPointer& update)
        {
              _prj = update;
        }
       , [this](const mccmsg::DevReqPtr&)
        {
            return;
        }
       , [this](const mccmsg::CmdCalibrationStartPtr& msg)
        {
            execute(*msg);
        }
       , [this](const mccmsg::CmdCalibrationCancelPtr& msg)
        {
            execute(*msg);
        }
       , [this](rc_channels_atom, photongen::fcu::RcChannels& channels)
         {
             handleChannelsMessage(channels);
         }
    };
}

const char* TraitRadioCalibration::name() const
{
    return _name.c_str();
}

void TraitRadioCalibration::handleChannelsMessage(const photongen::fcu::RcChannels& channels)
{
    auto copyFn = [](uint8_t channel, uint16_t value, uint8_t maxCount) -> bmcl::Option<uint16_t>
    {
        if (channel < maxCount && value != UINT16_MAX)
            return value;
        return bmcl::None;
    };

    _calibrationStatus.pwmValues[0]  = copyFn(0, channels.chanRaw()[0], channels.chanCount());
    _calibrationStatus.pwmValues[1]  = copyFn(1, channels.chanRaw()[1], channels.chanCount());
    _calibrationStatus.pwmValues[2]  = copyFn(2, channels.chanRaw()[2], channels.chanCount());
    _calibrationStatus.pwmValues[3]  = copyFn(3, channels.chanRaw()[3], channels.chanCount());
    _calibrationStatus.pwmValues[4]  = copyFn(4, channels.chanRaw()[4], channels.chanCount());
    _calibrationStatus.pwmValues[5]  = copyFn(5, channels.chanRaw()[5], channels.chanCount());
    _calibrationStatus.pwmValues[6]  = copyFn(6, channels.chanRaw()[6], channels.chanCount());
    _calibrationStatus.pwmValues[7]  = copyFn(7, channels.chanRaw()[7], channels.chanCount());
    _calibrationStatus.pwmValues[8]  = copyFn(8, channels.chanRaw()[8], channels.chanCount());
    _calibrationStatus.pwmValues[9]  = copyFn(9, channels.chanRaw()[9], channels.chanCount());
    _calibrationStatus.pwmValues[10] = copyFn(10, channels.chanRaw()[10], channels.chanCount());
    _calibrationStatus.pwmValues[11] = copyFn(11, channels.chanRaw()[11], channels.chanCount());
    _calibrationStatus.pwmValues[12] = copyFn(12, channels.chanRaw()[12], channels.chanCount());
    _calibrationStatus.pwmValues[13] = copyFn(13, channels.chanRaw()[13], channels.chanCount());
    _calibrationStatus.pwmValues[14] = copyFn(14, channels.chanRaw()[14], channels.chanCount());
    _calibrationStatus.pwmValues[15] = copyFn(15, channels.chanRaw()[15], channels.chanCount());
    _calibrationStatus.pwmValues[16] = copyFn(16, channels.chanRaw()[16], channels.chanCount());
    _calibrationStatus.pwmValues[17] = copyFn(17, channels.chanRaw()[17], channels.chanCount());

    sendTm();

    if (!_started)
        return;

    processChannelsMessage(*const_cast<photongen::fcu::RcChannels*>(&channels));
}

void TraitRadioCalibration::execute(/*CmdPtr&& cmd, */const mccmsg::CmdParamList& )
{

}

void TraitRadioCalibration::execute(/*CmdPtr&& cmd, */const mccmsg::CmdCalibrationStart& msg)
{
    if (msg.specialCmd().isNone())
    {
        using mccmsg::TmCalibration;

        if (!_activated)
        {
            return;
        }

        _startCalibration();

        _calibrationStatus.reset();
        _calibrationStatus._sensor = mccmsg::CalibrationSensor::Radio;
        _started = true;
        return;
    }

    switch (*msg.specialCmd())
    {
        case mccmsg::CalibrationCmd::NextStep:
            nextStep();
            break;
        case mccmsg::CalibrationCmd::SkipStep:
            skipStep();
            break;
        case mccmsg::CalibrationCmd::SetFlightModes:
        {
            auto modes = msg.flightModes().unwrap();
            photongen::fcu::RcChannel modeChannel = photongen::fcu::RcChannel::Unassigned;
            photongen::fcu::RcChannel returnChannel = photongen::fcu::RcChannel::Unassigned;
            photongen::fcu::RcChannel killSwitchChannel = photongen::fcu::RcChannel::Unassigned;
            photongen::fcu::RcChannel offboardSwitchChannel = photongen::fcu::RcChannel::Unassigned;
            std::array<photongen::fcu::FlightMode, 6> flightModes =
            {
                photongen::fcu::FlightMode::Unassigned,
                photongen::fcu::FlightMode::Unassigned,
                photongen::fcu::FlightMode::Unassigned,
                photongen::fcu::FlightMode::Unassigned,
                photongen::fcu::FlightMode::Unassigned,
                photongen::fcu::FlightMode::Unassigned
            };

            if (modes.flightModeChannel.isSome())
                modeChannel = (photongen::fcu::RcChannel)modes.flightModeChannel.unwrap();

            if (modes.returnSwitchChannel.isSome())
                returnChannel = (photongen::fcu::RcChannel)modes.returnSwitchChannel.unwrap();

            if (modes.killSwitchChannel.isSome())
                killSwitchChannel = (photongen::fcu::RcChannel)modes.killSwitchChannel.unwrap();

            if (modes.offboardSwitchChannel.isSome())
                offboardSwitchChannel = (photongen::fcu::RcChannel)modes.offboardSwitchChannel.unwrap();

            if (modes.mode1.isSome())
                flightModes[0] = (photongen::fcu::FlightMode)modes.mode1.unwrap();

            if (modes.mode2.isSome())
                flightModes[1] = (photongen::fcu::FlightMode)modes.mode2.unwrap();

            if (modes.mode3.isSome())
                flightModes[2] = (photongen::fcu::FlightMode)modes.mode3.unwrap();

            if (modes.mode4.isSome())
                flightModes[3] = (photongen::fcu::FlightMode)modes.mode4.unwrap();

            if (modes.mode5.isSome())
                flightModes[4] = (photongen::fcu::FlightMode)modes.mode5.unwrap();

            if (modes.mode6.isSome())
                flightModes[5] = (photongen::fcu::FlightMode)modes.mode6.unwrap();

            bmcl::Buffer dest;
            ::photon::CoderState state(::photon::OnboardTime::now());
            if (!_prj->interface()->encodeCmdFcuSetSimpleFlightModes(modeChannel, returnChannel, killSwitchChannel, offboardSwitchChannel, flightModes, &dest, &state))
            {
                BMCL_CRITICAL() << "Can't encodeCmdFcuSetSimpleFlightModes!";
                return;
            }

            bmcl::Bytes pkt = dest.asBytes();
            send(_broker, mccnet::send_cmd_atom::value, bmcl::SharedBytes::create(pkt.data(), pkt.size()));


        }
    }
}

void TraitRadioCalibration::execute(const mccmsg::CmdCalibrationCancel&)
{

}

//}
    //
    //if (_px4Vehicle()) {
    //    // If the RC_CHAN_COUNT parameter is available write the channel count
    //    if (parameterExists(FactSystem::defaultComponentId, "RC_CHAN_CNT")) {
    //        getParameterFact(FactSystem::defaultComponentId, "RC_CHAN_CNT")->setRawValue(_chanCount);
    //    }
    //}
    //
    //_stopCalibration();
    //_setInternalCalibrationValuesFromParameters();


void TraitRadioCalibration::_resetInternalCalibrationValues(void)
{
    // Set all raw channels to not reversed and center point values
    for (int i = 0; i < _chanMaxPX4; i++) {
        struct ChannelInfo* info = &_rgChannelInfo[i];
        info->function = rcCalFunctionMax;
        info->reversed = false;
        info->rcMin = TraitRadioCalibration::_rcCalPWMCenterPoint;
        info->rcMax = TraitRadioCalibration::_rcCalPWMCenterPoint;
        info->rcTrim = TraitRadioCalibration::_rcCalPWMCenterPoint;
    }

    // Initialize attitude function mapping to function channel not set
    for (size_t i = 0; i < rcCalFunctionMax; i++) {
        _rgFunctionChannelMapping[i] = _chanMaxPX4;
    }

//    _signalAllAttiudeValueChanges();
}

void TraitRadioCalibration::_setInternalCalibrationValuesFromParameters(void)
{
    // Initialize all function mappings to not set

//     for (int i = 0; i < _chanMax(); i++) {
//         struct ChannelInfo* info = &_rgChannelInfo[i];
//         info->function = rcCalFunctionMax;
//     }
//
//     for (size_t i = 0; i < rcCalFunctionMax; i++) {
//         _rgFunctionChannelMapping[i] = _chanMax();
//     }
//
//     // Pull parameters and update
//
//     QString minTpl("RC%1_MIN");
//     QString maxTpl("RC%1_MAX");
//     QString trimTpl("RC%1_TRIM");
//     QString revTpl("RC%1_REV");
//
//     bool convertOk;
//
//     for (int i = 0; i < _chanMax(); ++i) {
//         struct ChannelInfo* info = &_rgChannelInfo[i];
//
//         if (_px4Vehicle() && _apmPossibleMissingRCChannelParams.contains(i + 1)) {
//             if (!parameterExists(FactSystem::defaultComponentId, minTpl.arg(i + 1))) {
//                 // Parameter is missing from this version of APM
//                 info->rcTrim = 1500;
//                 info->rcMin = 1100;
//                 info->rcMax = 1900;
//                 info->reversed = false;
//                 continue;
//             }
//         }
//
//         Fact* paramFact = getParameterFact(FactSystem::defaultComponentId, trimTpl.arg(i + 1));
//         if (paramFact) {
//             info->rcTrim = paramFact->rawValue().toInt(&convertOk);
//             Q_ASSERT(convertOk);
//         }
//
//         paramFact = getParameterFact(FactSystem::defaultComponentId, minTpl.arg(i + 1));
//         if (paramFact) {
//             info->rcMin = paramFact->rawValue().toInt(&convertOk);
//             Q_ASSERT(convertOk);
//         }
//
//         paramFact = getParameterFact(FactSystem::defaultComponentId, maxTpl.arg(i + 1));
//         if (paramFact) {
//             info->rcMax = getParameterFact(FactSystem::defaultComponentId, maxTpl.arg(i + 1))->rawValue().toInt(&convertOk);
//             Q_ASSERT(convertOk);
//         }
//
//         paramFact = getParameterFact(FactSystem::defaultComponentId, revTpl.arg(i + 1));
//         if (paramFact) {
//             float floatReversed = paramFact->rawValue().toFloat(&convertOk);
//             Q_ASSERT(convertOk);
//             Q_ASSERT(floatReversed == 1.0f || floatReversed == -1.0f);
//             info->reversed = floatReversed == -1.0f;
//         }
//     }
//
//     for (int i = 0; i < rcCalFunctionMax; i++) {
//         int32_t paramChannel;
//
//         const char* paramName = _functionInfo()[i].parameterName;
//         if (paramName) {
//             Fact* paramFact = getParameterFact(FactSystem::defaultComponentId, paramName);
//             if (paramFact) {
//                 paramChannel = paramFact->rawValue().toInt(&convertOk);
//                 Q_ASSERT(convertOk);
//
//                 if (paramChannel != 0) {
//                     _rgFunctionChannelMapping[i] = paramChannel - 1;
//                     _rgChannelInfo[paramChannel - 1].function = (enum rcCalFunctions)i;
//                 }
//             }
//         }
//     }
}

void TraitRadioCalibration::_startCalibration(void)
{
    //Q_ASSERT(_chanCount >= _chanMinimum);

    _resetInternalCalibrationValues();

    // Let the mav known we are starting calibration. This should turn off motors and so forth.
//     if (_px4Vehicle()) {
//         _uas->startCalibration(UASInterface::StartCalibrationRadio);
//     }

    _calibrationStatus.nextEnabled = false;

    _currentStep = 0;
    _setupCurrentState();
    sendTm();
}

void TraitRadioCalibration::_stopCalibration(void)
{
    _currentStep = -1;

//    if (_uas) {
//        _uas->stopCalibration();
//        _setInternalCalibrationValuesFromParameters();
//    }
//    else {
//        _resetInternalCalibrationValues();
//    }

//    _statusText->setProperty("text", "");
//
//    _nextButton->setProperty("text", "Calibrate");
//    _nextButton->setEnabled(true);
//    _cancelButton->setEnabled(false);
//    _skipButton->setEnabled(false);

//    _setHelpImage(_imageCenter);
}

void TraitRadioCalibration::_rcCalSave(void)
{
    _rcCalState = rcCalStateSave;

    //_statusText->setProperty("text",
    //                         "The current calibration settings are now displayed for each channel on screen.\n\n"
    //                         "Click the Next button to upload calibration to board. Click Cancel if you don't want to save these values.");
    //
    _calibrationStatus.nextEnabled = true;
    _calibrationStatus.skipEnabled = false;
    //_cancelButton->setEnabled(true);

    // This updates the internal values according to the validation rules. Then _updateView will tick and update ui
    // such that the settings that will be written our are displayed.
    _validateCalibration();
    sendTm();
}

void TraitRadioCalibration::_rcCalSaveCurrentValues(void)
{
    for (int i = 0; i < _chanMaxPX4; i++) {
        _rcValueSave[i] = _rcRawValue[i];
        BMCL_DEBUG() << "_rcCalSaveCurrentValues channel:value" << i << _rcValueSave[i];
    }
}

void TraitRadioCalibration::_setHelpImage(const char* imageFile)
{
    _calibrationStatus.image = imageFile;
}

void TraitRadioCalibration::nextStep()
{
    if (_currentStep == -1) {
        // Need to have enough channels
        if (_chanCount < _chanMinimum) {
            return;
        }
        _startCalibration();
    }
    else {
        const stateMachineEntry* state = _getStateMachineEntry(_currentStep);
        Q_ASSERT(state);
        Q_ASSERT(state->nextFn);
        (this->*state->nextFn)();
    }
}

void TraitRadioCalibration::skipStep()
{
    Q_ASSERT(_currentStep != -1);

    const stateMachineEntry* state = _getStateMachineEntry(_currentStep);
    Q_ASSERT(state);
    Q_ASSERT(state->skipFn);
    (this->*state->skipFn)();
}

void TraitRadioCalibration::sendTm()
{
    _helper.sendTrait(new mccmsg::TmCalibration(_id.device(), _calibrationStatus));
}
}
