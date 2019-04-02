#pragma once

#include <caf/atom.hpp>
#include <caf/event_based_actor.hpp>

#include "mcc/net/Cmd.h"
#include "mcc/net/TmHelper.h"

#include "mcc/msg/Calibration.h"

#include "../device/Mavlink.h"
#include "../device/MavlinkUtils.h"
#include "../traits/TraitRadioCalibration.h"

namespace mccmav {

struct CalibrationVariables
{
    CalibrationVariables()
    {
        CAL_MAG0_ID = 0;
        CAL_GYRO0_ID = 0;
        CAL_ACC0_ID = 0;
        SENS_BOARD_X_OFF = 0;

        RC_MAP_ROLL = 0;
        RC_MAP_PITCH = 0;
        RC_MAP_YAW = 0;
        RC_MAP_THROTTLE = 0;
    }

    int32_t CAL_MAG0_ID;
    int32_t CAL_GYRO0_ID;
    int32_t CAL_ACC0_ID;
    int32_t SENS_BOARD_X_OFF;

    int32_t RC_MAP_ROLL;
    int32_t RC_MAP_PITCH;
    int32_t RC_MAP_YAW;
    int32_t RC_MAP_THROTTLE;
};

class TraitSensorCalibration : public caf::event_based_actor
{
    friend class CalibrationCmdsVisitor;

public:
    TraitSensorCalibration(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const mccmsg::ProtocolId& id, const std::string& name, const MavlinkSettings& settings);
    caf::behavior make_behavior() override;
    const char* name() const override;

private:
    void execute(const mccmsg::CmdParamList& msg);
    void execute(const mccmsg::CmdCalibrationStart& msg);
    void execute(const mccmsg::CmdCalibrationCancel& msg);

    void handleTextMessage(const std::string& text);

    void extractIntFromString(const std::string& text, const std::string& start, const std::string& stop) const;
private:
    mccnet::TmHelper _helper;
    mccmsg::ProtocolId _id;

    std::string _name;
    caf::actor _core;
    caf::actor _broker;

    bool _activated;

    bool _calibrationStarted;
    MavlinkSettings _settings;

    mccmsg::Calibration _calibrationStatus;
    mccmsg::Calibration _flightModesCalibration;

    CalibrationVariables _calibrationVars;
};
}
