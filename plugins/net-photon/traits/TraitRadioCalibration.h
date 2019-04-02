#pragma once
#include <caf/atom.hpp>
#include <caf/event_based_actor.hpp>

#include "mcc/net/Cmd.h"
#include "mcc/net/TmHelper.h"
#include "mcc/net/Timer.h"

#include "mcc/msg/Calibration.h"

#include <bmcl/MemReader.h>
#include <decode/parser/Project.h>

#include <photongen/groundcontrol/fcu/RcChannels.hpp>
#include <photon/groundcontrol/GroundControl.h>
#include <photon/groundcontrol/ProjectUpdate.h>

namespace mccphoton {

class TraitRadioCalibration : public caf::event_based_actor
{
    friend class CalibrationCmdsVisitor;

public:
    /// @brief These identify the various controls functions. They are also used as indices into the _rgFunctioInfo
    /// array.
    enum rcCalFunctions {
        rcCalFunctionRoll,
        rcCalFunctionPitch,
        rcCalFunctionYaw,
        rcCalFunctionThrottle,
        rcCalFunctionMax,
    };

    /// @brief The states of the calibration state machine.
    enum rcCalStates {
        rcCalStateChannelWait,
        rcCalStateBegin,
        rcCalStateIdentify,
        rcCalStateMinMax,
        rcCalStateCenterThrottle,
        rcCalStateDetectInversion,
        rcCalStateTrims,
        rcCalStateSave
    };

    struct ChannelInfo {
        rcCalFunctions function;   ///< Function mapped to this channel, rcCalFunctionMax for none
        bool                reversed;   ///< true: channel is reverse, false: not reversed
        int                 rcMin;      ///< Minimum RC value
        int                 rcMax;      ///< Maximum RC value
        int                 rcTrim;     ///< Trim position
    };

    typedef void (TraitRadioCalibration::*inputFn)(rcCalFunctions function, int chan, int value);
    typedef void (TraitRadioCalibration::*buttonFn)(void);

    struct stateMachineEntry {
        rcCalFunctions function;
        const char*         instructions;
        const char*         image;
        inputFn             rcInputFn;
        buttonFn            nextFn;
        buttonFn            skipFn;
    };

    /// @brief A set of information associated with a function.
    struct FunctionInfo {
        const char* parameterName;  ///< Parameter name for function mapping
    };

    TraitRadioCalibration(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const mccmsg::ProtocolId& id, const std::string& name);

    caf::behavior make_behavior() override;
    const char* name() const override;

    void execute(const mccmsg::CmdParamList& msg);
    void execute(const mccmsg::CmdCalibrationStart& msg);
    void execute(const mccmsg::CmdCalibrationCancel& msg);

    void processChannelsMessage(photongen::fcu::RcChannels& channels);

    const stateMachineEntry* _getStateMachineEntry(int step) const;
    const FunctionInfo* _functionInfo(void) const;

    void _advanceState(void);
    void _setupCurrentState(void);

    void _inputCenterWaitBegin(enum rcCalFunctions function, int channel, int value);
    void _inputStickDetect(enum rcCalFunctions function, int channel, int value);
    void _inputStickMin(enum rcCalFunctions function, int channel, int value);
    void _inputCenterWait(enum rcCalFunctions function, int channel, int value);
    void _inputSwitchMinMax(enum rcCalFunctions function, int channel, int value);
    void _inputSwitchDetect(enum rcCalFunctions function, int channel, int value);

    void _switchDetect(enum rcCalFunctions function, int channel, int value, bool moveToNextStep);

    void _saveAllTrims(void);

    bool _stickSettleComplete(int value);

    void _validateCalibration(void);
    void _writeCalibration(void);
    void _resetInternalCalibrationValues(void);
    void _setInternalCalibrationValuesFromParameters(void);

    void _startCalibration(void);
    void _stopCalibration(void);
    void _rcCalSave(void);

    void _rcCalSaveCurrentValues(void);

    void _setHelpImage(const char* imageFile);

    void nextStep();
    void skipStep();

private:
    void handleChannelsMessage(const photongen::fcu::RcChannels& channels);

    void sendTm();

    int _currentStep;  ///< Current step of state machine
    static const struct FunctionInfo _rgFunctionInfoPX4[rcCalFunctionMax];
    int _rgFunctionChannelMapping[rcCalFunctionMax];

    static constexpr int _attitudeControls = 5;

    int _chanCount;                     ///< Number of actual rc channels available
    static constexpr int _chanMaxPX4 = 18;  ///< Maximum number of supported rc channels, PX4 Firmware
    static constexpr int _chanMaxAPM = 14;  ///< Maximum number of supported rc channels, APM firmware
    static constexpr int _chanMaxAny = 18;  ///< Maximum number of support rc channels by this implementation
    static constexpr int _chanMinimum = 5;  ///< Minimum number of channels required to run

    struct ChannelInfo _rgChannelInfo[_chanMaxAny];    ///< Information associated with each rc channel

    QList<int> _apmPossibleMissingRCChannelParams;  ///< List of possible missing RC*_* params for APM stack

    enum rcCalStates _rcCalState;       ///< Current calibration state
    int _rcCalStateCurrentChannel;      ///< Current channel being worked on in rcCalStateIdentify and rcCalStateDetectInversion
    bool _rcCalStateChannelComplete;    ///< Work associated with current channel is complete
    int _rcCalStateIdentifyOldMapping;  ///< Previous mapping for channel being currently identified
    int _rcCalStateReverseOldMapping;   ///< Previous mapping for channel being currently used to detect inversion


    //const int RadioCalibration::_updateInterval = 150;              ///< Interval for timer which updates radio channel widgets
    // FIXME: Double check these mins againt 150% throws
    static constexpr int _rcCalPWMValidMinValue = 1300;      ///< Largest valid minimum PWM Min range value
    static constexpr int _rcCalPWMValidMaxValue = 1700;      ///< Smallest valid maximum PWM Max range value
    static constexpr int _rcCalPWMCenterPoint = ((_rcCalPWMValidMaxValue - _rcCalPWMValidMinValue) / 2.0f) + _rcCalPWMValidMinValue;
    static constexpr int _rcCalPWMDefaultMinValue = 1000;    ///< Default value for Min if not set
    static constexpr int _rcCalPWMDefaultMaxValue = 2000;    ///< Default value for Max if not set
    static constexpr int _rcCalRoughCenterDelta = 50;        ///< Delta around center point which is considered to be roughly centered
    static constexpr int _rcCalMoveDelta = 300;            ///< Amount of delta past center which is considered stick movement
    static constexpr int _rcCalSettleDelta = 20;           ///< Amount of delta which is considered no stick movement
    static constexpr int _rcCalMinDelta = 100;             ///< Amount of delta allowed around min value to consider channel at min


    int _rcValueSave[_chanMaxAny];        ///< Saved values prior to detecting channel movement

    int _rcRawValue[_chanMaxAny];         ///< Current set of raw channel values

    int     _stickDetectChannel;
    int     _stickDetectInitialValue;
    int     _stickDetectValue;
    bool    _stickDetectSettleStarted;
    mccnet::Timer   _stickDetectSettleElapsed;

    mccmsg::Calibration _calibrationStatus;

    ::photon::ProjectUpdate::ConstPointer _prj;

    mccnet::TmHelper _helper;
    mccmsg::ProtocolId _id;

    std::string _name;
    caf::actor _core;
    caf::actor _broker;

    bool _activated;
    bool _started;
};
}
