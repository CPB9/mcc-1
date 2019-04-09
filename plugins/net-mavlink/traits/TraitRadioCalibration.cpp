#include "../traits/TraitRadioCalibration.h"
#include "../traits/Trait.h"
#include "Firmware.h"
#include "mcc/msg/FwdExt.h"
#include <bmcl/Logging.h>
#include <bmcl/MakeRc.h>
#include <fmt/format.h>
#include <array>

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmav::MavlinkMessagePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mavlink_rc_channels_t);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NotificationPtr);

//
// class CalibrationCmdsVisitor1 : public mccmsg::CmdVisitor
// {
// public:
//     CalibrationCmdsVisitor1(RadioCalibration* self, mccnet::CmdPtr&& cmd)
//         : mccmsg::CmdVisitor([&](const mccmsg::cmd::Any_Request*)
//     {
//         //assert(false);
//         _cmd->sendFailed(mccmsg::Error::CmdUnknown);
//     })
//         , _self(self)
//         , _cmd(std::move(cmd))
//     {
//     }
//     void visit(const mccmsg::CmdCalibrationStart* msg) override { _self->execute(std::move(_cmd), *msg); }
//     void visit(const mccmsg::CmdCalibrationCancel* msg) override { _self->execute(std::move(_cmd), *msg); }
//
// private:
//     mccnet::CmdPtr _cmd;
//     RadioCalibration* _self;
// };

namespace mccmav {

constexpr std::chrono::milliseconds _stickDetectSettle = std::chrono::milliseconds(500);

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

void TraitRadioCalibration::writeParameter(const QString& id, float value)
{
    mavlink_param_union_t paramUnion;
    paramUnion.param_float = value;
    paramUnion.type = MAV_PARAM_TYPE_REAL32;

    mavlink_param_set_t     p;
    p.param_value = paramUnion.param_float;
    p.param_type = paramUnion.type;
    p.target_system = _settings.system;
    p.target_component = (uint8_t)_settings.component;

    strncpy(p.param_id, id.toStdString().c_str(), sizeof(p.param_id));

    MavlinkMessageRc* msg = new MavlinkMessageRc;
    mavlink_msg_param_set_encode_chan(_settings.system,
                                      _settings.component,
                                      _settings.channel,
                                      msg,
                                      &p);

    request(_broker, caf::infinite, send_msg_atom::value, MavlinkMessagePtr(msg)).then(
        [this]()
        {
        }
        , [this](const caf::error& )
        {
        }
    );
}


void TraitRadioCalibration::writeParameterInt(const QString& id, int value)
{
    mavlink_param_union_t paramUnion;
    paramUnion.param_int32 = value;
    paramUnion.type = MAV_PARAM_TYPE_INT32;

    mavlink_param_set_t     p;
    p.param_value = paramUnion.param_float;
    p.param_type = paramUnion.type;
    p.target_system = _settings.system;
    p.target_component = (uint8_t)_settings.component;

    strncpy(p.param_id, id.toStdString().c_str(), sizeof(p.param_id));

    MavlinkMessageRc* msg = new MavlinkMessageRc;
    mavlink_msg_param_set_encode_chan(_settings.system,
                                      _settings.component,
                                      _settings.channel,
                                      msg,
                                      &p);

    request(_broker, caf::infinite, send_msg_atom::value, MavlinkMessagePtr(msg)).then(
        [this]()
    {
    }
        , [this](const caf::error& err)
    {
    }
    );
}

const struct TraitRadioCalibration::FunctionInfo TraitRadioCalibration::_rgFunctionInfoPX4[TraitRadioCalibration::rcCalFunctionMax] = {
    { "RC_MAP_ROLL" },
    { "RC_MAP_PITCH" },
    { "RC_MAP_YAW" },
    { "RC_MAP_THROTTLE" }
};

TraitRadioCalibration::TraitRadioCalibration(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const mccmsg::ProtocolId& id, const std::string& name, const MavlinkSettings& settings)
    : caf::event_based_actor(cfg), _helper(id, core, this), _id(id), _activated(false), _name(name), _core{ core }, _broker{ broker }, _settings(settings)
    , _currentStep(-1)
    , _chanCount(0)
    , _rcCalState(rcCalStateChannelWait)
    , _started(false)
    , _cancelCalibrationCounter(0)
{
    _resetInternalCalibrationValues();
    _calibrationStatus._sensor = mccmsg::CalibrationSensor::Radio;
}

void TraitRadioCalibration::processChannelsMessage(mavlink_rc_channels_t& channels)
{
    int pwmValues[_chanMaxPX4];

    auto getPwm = [&](int i, uint16_t channelValue) {
        if (i < channels.chancount) {
            pwmValues[i] = channelValue == UINT16_MAX ? -1 : channelValue;
        }
        else {
            pwmValues[i] = -1;
        }
    };

    getPwm(0, channels.chan1_raw);
    getPwm(1, channels.chan2_raw);
    getPwm(2, channels.chan3_raw);
    getPwm(3, channels.chan4_raw);
    getPwm(4, channels.chan5_raw);
    getPwm(5, channels.chan6_raw);
    getPwm(6, channels.chan7_raw);
    getPwm(7, channels.chan8_raw);
    getPwm(8, channels.chan8_raw);
    getPwm(9, channels.chan10_raw);
    getPwm(10, channels.chan11_raw);
    getPwm(11, channels.chan12_raw);
    getPwm(12, channels.chan13_raw);
    getPwm(13, channels.chan14_raw);
    getPwm(14, channels.chan15_raw);
    getPwm(15, channels.chan16_raw);
    getPwm(16, channels.chan17_raw);
    getPwm(17, channels.chan18_raw);

    /////////////////////////////////////////////
    int maxChannel = channels.chancount;


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

    QString minTpl("RC%1_MIN");
    QString maxTpl("RC%1_MAX");
    QString trimTpl("RC%1_TRIM");
    QString revTpl("RC%1_REV");

    // Note that the rc parameters are all float, so you must cast to float in order to get the right QVariant
    for (int chan = 0; chan < _chanMaxPX4; chan++) {
        struct ChannelInfo* info = &_rgChannelInfo[chan];
        int                 oneBasedChannel = chan + 1;

        writeParameter(trimTpl.arg(oneBasedChannel), info->rcTrim);
        writeParameter(minTpl.arg(oneBasedChannel), info->rcMin);
        writeParameter(maxTpl.arg(oneBasedChannel), info->rcMax);

        float reversedParamValue = info->reversed ? -1.0f : 1.0f;
        writeParameter(revTpl.arg(oneBasedChannel), reversedParamValue);
    }

    // Write function mapping parameters
    for (size_t i = 0; i < rcCalFunctionMax; i++)
    {
        int32_t paramChannel;
        if (_rgFunctionChannelMapping[i] == _chanMaxPX4)
        {
            // 0 signals no mapping
            paramChannel = 0;
        }
        else
        {
            // Note that the channel value is 1-based
            paramChannel = _rgFunctionChannelMapping[i] + 1;
        }
        const char* paramName = _functionInfo()[i].parameterName;
        if (paramName)
        {
            writeParameterInt(paramName, paramChannel);
        }
    }

    _calibrationStatus.done = true;
    _calibrationStatus.nextEnabled = false;
    _calibrationStatus.skipEnabled = false;
    _calibrationStatus.message = "Calibration completed!";

    _started = false;
    _stopCalibration();
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
      , [this](const mccmsg::DevReqPtr&)
        {
            return;
//            CalibrationCmdsVisitor1 visitor{ this, bmcl::makeRc<mccnet::Cmd>(make_response_promise(), msg) };
//            msg->visit(&visitor);
        }
       , [this](const mccmsg::CmdCalibrationStartPtr& msg)
        {
            execute(*msg);
        }
       , [this](const mccmsg::CmdCalibrationCancelPtr& msg)
        {
            execute(*msg);
        }
      , [this](const mavlink_rc_channels_t& channels)
        {
            handleChannelsMessage(channels);
        }
      ,[this](frmupdate_atom, const FirmwarePtr& frm)
        {

        }
    };
}

const char* TraitRadioCalibration::name() const
{
    return _name.c_str();
}

void TraitRadioCalibration::handleChannelsMessage(const mavlink_rc_channels_t& channels)
{
    auto copyFn = [](uint8_t channel, uint16_t value, uint8_t maxCount) -> bmcl::Option<uint16_t>
    {
        if (channel < maxCount && value != UINT16_MAX)
            return value;
        return bmcl::None;
    };

    _calibrationStatus.pwmValues[0] = copyFn(0, channels.chan1_raw, channels.chancount);
    _calibrationStatus.pwmValues[1] = copyFn(1, channels.chan2_raw, channels.chancount);
    _calibrationStatus.pwmValues[2] = copyFn(2, channels.chan3_raw, channels.chancount);
    _calibrationStatus.pwmValues[3] = copyFn(3, channels.chan4_raw, channels.chancount);
    _calibrationStatus.pwmValues[4] = copyFn(4, channels.chan5_raw, channels.chancount);
    _calibrationStatus.pwmValues[5] = copyFn(5, channels.chan6_raw, channels.chancount);
    _calibrationStatus.pwmValues[6] = copyFn(6, channels.chan7_raw, channels.chancount);
    _calibrationStatus.pwmValues[7] = copyFn(7, channels.chan8_raw, channels.chancount);
    _calibrationStatus.pwmValues[8] = copyFn(8, channels.chan9_raw, channels.chancount);
    _calibrationStatus.pwmValues[9] = copyFn(9, channels.chan10_raw, channels.chancount);
    _calibrationStatus.pwmValues[10] = copyFn(10, channels.chan11_raw, channels.chancount);
    _calibrationStatus.pwmValues[11] = copyFn(11, channels.chan12_raw, channels.chancount);
    _calibrationStatus.pwmValues[12] = copyFn(12, channels.chan13_raw, channels.chancount);
    _calibrationStatus.pwmValues[13] = copyFn(13, channels.chan14_raw, channels.chancount);
    _calibrationStatus.pwmValues[14] = copyFn(14, channels.chan15_raw, channels.chancount);
    _calibrationStatus.pwmValues[15] = copyFn(15, channels.chan16_raw, channels.chancount);
    _calibrationStatus.pwmValues[16] = copyFn(16, channels.chan17_raw, channels.chancount);
    _calibrationStatus.pwmValues[17] = copyFn(17, channels.chan18_raw, channels.chancount);

    sendTm();

    if (!_started)
        return;

    processChannelsMessage(*const_cast<mavlink_rc_channels_t*>(&channels));
}

void TraitRadioCalibration::execute(const mccmsg::CmdParamList& )
{

}

void TraitRadioCalibration::execute(const mccmsg::CmdCalibrationStart& msg)
{
    if (msg.component() != mccmsg::CalibrationSensor::Radio)
        return;

    if (msg.specialCmd().isNone())
    {
        using mccmsg::CmdCalibrationStart;
        using mccmsg::TmCalibration;

        if (!_activated)
        {
            return;
            //        cmd->sendFailed();
        }

        int gyroCal = 0;
        int magCal = 0;
        int airspeedCal = 0;
        int radioCal = 1;
        int accelCal = 0;
        int escCal = 0;

        MavlinkMessageRc* mavMsg = new MavlinkMessageRc;
        mavlink_msg_command_long_pack_chan(_settings.system,
                                           _settings.component,
                                           _settings.channel,
                                           mavMsg,
                                           (uint8_t)_id.id(),
                                           MAV_COMP_ID_ALL,   // target component
                                           MAV_CMD_PREFLIGHT_CALIBRATION,    // command id
                                           0,                                // 0=first transmission of command
                                           gyroCal,                          // gyro cal
                                           magCal,                           // mag cal
                                           0,                                // ground pressure
                                           radioCal,                         // radio cal
                                           accelCal,                         // accel cal
                                           airspeedCal,                      // PX4: airspeed cal, ArduPilot: compass mot
                                           escCal);                          // esc cal


        request(_broker, caf::infinite, send_msg_atom::value, MavlinkMessagePtr(mavMsg)).then(
            [this]()
            {
            }
          , [this](const caf::error&)
            {
            }
        );

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

        if(modes.mode1.isSome())
            writeParameterInt("COM_FLTMODE1", (int)modes.mode1.unwrap());
        if (modes.mode2.isSome())
            writeParameterInt("COM_FLTMODE2", (int)modes.mode2.unwrap());
        if (modes.mode3.isSome())
            writeParameterInt("COM_FLTMODE3", (int)modes.mode3.unwrap());
        if (modes.mode4.isSome())
            writeParameterInt("COM_FLTMODE4", (int)modes.mode4.unwrap());
        if (modes.mode5.isSome())
            writeParameterInt("COM_FLTMODE5", (int)modes.mode5.unwrap());
        if (modes.mode6.isSome())
            writeParameterInt("COM_FLTMODE6", (int)modes.mode6.unwrap());
        if (modes.returnSwitchChannel.isSome())
            writeParameterInt("RC_MAP_RETURN_SW", (int)modes.returnSwitchChannel.unwrap());
        if (modes.killSwitchChannel.isSome())
            writeParameterInt("RC_MAP_KILL_SW", (int)modes.killSwitchChannel.unwrap());
        if (modes.offboardSwitchChannel.isSome())
            writeParameterInt("RC_MAP_OFFB_SW", (int)modes.offboardSwitchChannel.unwrap());
        if (modes.flightModeChannel .isSome())
            writeParameterInt("RC_MAP_FLTMODE", (int)modes.flightModeChannel.unwrap());
    }
    }
}

void TraitRadioCalibration::execute(const mccmsg::CmdCalibrationCancel&)
{
    if (!_started)
        return;

    _cancelCalibrationCounter = 3;
    _stopCalibration();
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

    MavlinkMessageRc* msg = new MavlinkMessageRc;
    mavlink_msg_command_long_pack_chan(_settings.system,
                                       _settings.component,
                                       _settings.channel,
                                       msg,
                                       (uint8_t)_id.id(),
                                       MAV_COMP_ID_ALL,   // target component
                                       MAV_CMD_PREFLIGHT_CALIBRATION,    // command id
                                       0,                                // 0=first transmission of command
                                       0,                          // gyro cal
                                       0,                           // mag cal
                                       0,                                // ground pressure
                                       0,                         // radio cal
                                       0,                         // accel cal
                                       0,                      // PX4: airspeed cal, ArduPilot: compass mot
                                       0);                          // esc cal

    _helper.log_text(bmcl::LogLevel::Info, "Остановка калибровки пульта...");

    request(_broker, std::chrono::seconds(3), waypoint_msg_atom::value, MavlinkMessagePtr(msg), MAVLINK_MSG_ID_COMMAND_ACK).then(
        [this](const MavlinkMessagePtr& message)
        {
            _cancelCalibrationCounter = 0;
            _helper.log_text(bmcl::LogLevel::Warning, "Калибровка отменена!");
}
        , [this](const caf::error& err)
        {
            if (_cancelCalibrationCounter > 0)
            {
                _helper.log(bmcl::LogLevel::Warning, fmt::format("Повтор команды отмены калибровки, осталось попыток: {}!", _cancelCalibrationCounter));
                _stopCalibration();
                _cancelCalibrationCounter--;
        }
            else
            {
                _helper.log_text(bmcl::LogLevel::Critical, "Нет квитанции об отмене калибровки!");
            }
        }
    );
//
//     request(_broker, caf::infinite, send_msg_atom::value, mavMsg).then(
//         [this]()
//         {
//         }
//       , [this](const caf::error&)
//         {
//         }
//     );
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
