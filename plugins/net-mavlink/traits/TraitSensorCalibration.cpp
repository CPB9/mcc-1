#include "../traits/TraitSensorCalibration.h"
#include "../traits/Trait.h"
#include "../device/Device.h"

#include <fmt/format.h>
#include <bmcl/Logging.h>
#include <bmcl/MakeRc.h>
#include <regex>

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmav::MavlinkMessagePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NotificationPtr);

namespace mccmav {

inline bool startsWith(const std::string& text, const std::string& pattern)
{
    if (pattern.size() > text.size())
        return false;
    return std::equal(pattern.begin(), pattern.end(), text.begin());
}

inline bool endsWith(const std::string& text, const std::string& pattern)
{
    if (pattern.size() > text.size())
        return false;
    return std::equal(pattern.rbegin(), pattern.rend(), text.rbegin());
}

inline bool contains(const std::string& text, const std::string& pattern)
{
    return text.find(pattern) != std::string::npos;
}
//
// class CalibrationCmdsVisitor : public mccmsg::CmdVisitor
// {
// public:
//     CalibrationCmdsVisitor(TraitCalibration* self, mccnet::CmdPtr&& cmd)
//         : mccmsg::CmdVisitor([&](const mccmsg::cmd::Any_Request*)
//           {
//               //assert(false);
//               _cmd->sendFailed(mccmsg::Error::CmdUnknown);
//           })
//         , _self(self)
//         , _cmd(std::move(cmd))
//     {
//     }
//     void visit(const mccmsg::CmdCalibrationStart* msg) override { _self->execute(std::move(_cmd), *msg); }
//     void visit(const mccmsg::CmdCalibrationCancel* msg) override { _self->execute(std::move(_cmd), *msg); }
//
// private:
//     mccnet::CmdPtr _cmd;
//     TraitCalibration* _self;
// };


TraitSensorCalibration::TraitSensorCalibration(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const mccmsg::ProtocolId& id, const std::string& name, const MavlinkSettings& settings)
    : caf::event_based_actor(cfg)
    , _helper(id, core, this)
    , _id(id)
    , _activated(false)
    , _calibrationStarted(false)
    , _name(name)
    , _core{ core }
    , _broker{ broker }
    , _settings(settings)
{
    _flightModesCalibration._sensor = mccmsg::CalibrationSensor::FlightModes;
}

caf::behavior TraitSensorCalibration::make_behavior()
{
    using timer_state_atom = caf::atom_constant<caf::atom("tmrsndcal")>;
    send(this, timer_state_atom::value);
    return
    {
        [this](timer_state_atom)
        {
            if (_activated)
            {
                mccmsg::TmCommonCalibrationStatus* _commonStatusPtr = new mccmsg::TmCommonCalibrationStatus(_id.device());
                mccmsg::TmCommonCalibrationStatus& _commonStatus = *_commonStatusPtr;
                // sensors calibration
                // CAL_MAG0_ID != 0 magnetometer
                // CAL_GYRO0_ID != 0 gyroscope
                // CAL_ACC0_ID != 0 accelerometer
                // SENS_BOARD_X_OFF != 0 horizont
                _commonStatus.magnetometer = (_calibrationVars.CAL_MAG0_ID != 0);
                _commonStatus.gyroscope = (_calibrationVars.CAL_GYRO0_ID != 0);
                _commonStatus.accelerometer = (_calibrationVars.CAL_ACC0_ID != 0);
                _commonStatus.level = (_calibrationVars.SENS_BOARD_X_OFF != 0);

                // radio calibration
                // RC_MAP_ROLL != 0
                // RC_MAP_PITCH != 0
                // RC_MAP_YAW != 0
                // RC_MAP_THROTTLE != 0
                _commonStatus.radio = (_calibrationVars.RC_MAP_ROLL != 0 &&
                                       _calibrationVars.RC_MAP_PITCH != 0 &&
                                       _calibrationVars.RC_MAP_YAW != 0 &&
                                       _calibrationVars.RC_MAP_THROTTLE != 0);

                _helper.sendTrait(_commonStatusPtr);
                _helper.sendTrait(new mccmsg::TmCalibration(_id.device(), _flightModesCalibration));
            }

            delayed_send(this, std::chrono::milliseconds(1000), timer_state_atom::value);
        }
        , [this](set_param_atom, const std::string& name, const mccmsg::NetVariant& value)
        {
            if (name == "CAL_MAG0_ID")
                _calibrationVars.CAL_MAG0_ID = value.toInt();
            else if (name == "CAL_GYRO0_ID")
                _calibrationVars.CAL_GYRO0_ID = value.toInt();
            else if (name == "CAL_ACC0_ID")
                _calibrationVars.CAL_ACC0_ID = value.toInt();
            else if (name == "SENS_BOARD_X_OFF")
                _calibrationVars.SENS_BOARD_X_OFF = value.toInt();
            else if (name == "RC_MAP_ROLL")
                _calibrationVars.RC_MAP_ROLL = value.toInt();
            else if (name == "RC_MAP_PITCH")
                _calibrationVars.RC_MAP_PITCH = value.toInt();
            else if (name == "RC_MAP_YAW")
                _calibrationVars.RC_MAP_YAW = value.toInt();
            else if (name == "RC_MAP_THROTTLE")
                _calibrationVars.RC_MAP_THROTTLE = value.toInt();
            else if (name == "RC_MAP_KILL_SW")
                _flightModesCalibration.flightModes.killSwitchChannel = value.toInt();
            else if (name == "RC_MAP_OFFB_SW")
                _flightModesCalibration.flightModes.offboardSwitchChannel = value.toInt();
            else if (name == "RC_MAP_RETURN_SW")
                _flightModesCalibration.flightModes.returnSwitchChannel = value.toInt();
            else if (name == "RC_MAP_FLTMODE")
                _flightModesCalibration.flightModes.flightModeChannel = value.toInt();
            else if (name == "COM_FLTMODE1")
                _flightModesCalibration.flightModes.mode1 = value.toInt();
            else if (name == "COM_FLTMODE2")
                _flightModesCalibration.flightModes.mode2 = value.toInt();
            else if (name == "COM_FLTMODE3")
                _flightModesCalibration.flightModes.mode3 = value.toInt();
            else if (name == "COM_FLTMODE4")
                _flightModesCalibration.flightModes.mode4 = value.toInt();
            else if (name == "COM_FLTMODE5")
                _flightModesCalibration.flightModes.mode5 = value.toInt();
            else if (name == "COM_FLTMODE6")
                _flightModesCalibration.flightModes.mode6 = value.toInt();
        }
        , [this](activated_atom)
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
//       , [this](const mccmsg::cmd::Any_RequestPtr& msg)
//         {
//             CalibrationCmdsVisitor visitor{ this, bmcl::makeRc<mccnet::Cmd>(make_response_promise(), msg) };
//             msg->visit(&visitor);
//         }
      , [this](const mccmsg::CmdCalibrationStartPtr& msg)
        {
            execute(*msg);
        }
      , [this](const mccmsg::CmdCalibrationCancelPtr& msg)
        {
            execute(*msg);
        }
      , [this](const std::string& text)
        {
            if (!_calibrationStarted)
                return;

            handleTextMessage(text);
        }
      , [this](frmupdate_atom, const FirmwarePtr& frm)
        {

        }
    };
}

const char* TraitSensorCalibration::name() const
{
    return _name.c_str();
}

void TraitSensorCalibration::execute(const mccmsg::CmdParamList& msg)
{
    //cmd->sendDone();
}

void TraitSensorCalibration::execute(const mccmsg::CmdCalibrationStart& msg)
{
    using mccmsg::CmdCalibrationStart;
    using mccmsg::TmCalibration;
    using mccmsg::CalibrationSensor;

    if (!_activated)
    {
//        cmd->sendFailed();
    }

    int gyroCal     = 0;
    int magCal      = 0;
    int airspeedCal = 0;
    int radioCal    = 0;
    int accelCal    = 0;
    int escCal      = 0;

    std::string sensor;
    switch (msg.component())
    {
    case CalibrationSensor::Magnetometer: magCal = 1; sensor = "mag"; break;
    case CalibrationSensor::Gyroscope: gyroCal = 1; sensor = "gyro"; break;
    case CalibrationSensor::Accelerometer: accelCal = 1; sensor = "accel"; break;
    case CalibrationSensor::Level: accelCal = 2; sensor = "level"; break;
    case CalibrationSensor::Radio: radioCal = 1; sensor = "radio"; break;
    case CalibrationSensor::Esc: escCal = 1; sensor = "esc"; break;
    }

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
                    [this, sensor]()
                    {
                        _helper.log(bmcl::LogLevel::Info, fmt::format("Калибровка сенсора: {}", sensor));
                    }
                  , [this](const caf::error& err)
                    {
                    }
                );

    _calibrationStarted = true;
    _calibrationStatus.reset();
    _calibrationStatus._sensor = msg.component();
    _helper.sendTrait(new mccmsg::TmCalibration(_id.device(), _calibrationStatus));
}

void TraitSensorCalibration::execute(const mccmsg::CmdCalibrationCancel& msg)
{
    _calibrationStatus.waitingForCancel = true;

    MavlinkMessageRc*       mavmsg = new MavlinkMessageRc;
    mavlink_command_long_t  mavcmd;

    mavcmd.command = MAV_CMD_PREFLIGHT_CALIBRATION;
    mavcmd.confirmation = 0;
    mavcmd.param1 = 0;
    mavcmd.param2 = 0;
    mavcmd.param3 = 0;
    mavcmd.param4 = 0;
    mavcmd.param5 = 0;
    mavcmd.param6 = 0;
    mavcmd.param7 = 0;
    mavcmd.target_system = (uint8_t)_id.id();
    mavcmd.target_component = MAV_COMP_ID_ALL;

    mavlink_msg_command_long_encode_chan(_settings.system,
                                         _settings.component,
                                         _settings.channel,
                                         mavmsg,
                                         &mavcmd);

    request(_broker, caf::infinite, send_msg_atom::value, MavlinkMessagePtr(mavmsg)).then(
        [this]()
        {
            _helper.log_text(bmcl::LogLevel::Warning, "Отмена калибровки сенсора");
        }
      , [this](const caf::error& err)
        {
        }
    );
//    cmd->sendDone();
}

void TraitSensorCalibration::handleTextMessage(const std::string& text)
{
    const std::string CAL_PREFIX_MSG                 = "[cal] ";
    const std::regex CAL_QGC_STARTED_MSG              ("\\[cal\\] calibration started: ([\\d]) ([\\w]+)");                     // "[cal] calibration started: 2 %s"
    const std::regex CAL_QGC_DONE_MSG                 ("\\[cal\\] calibration done: ([\\w\\s]+)");                              // "[cal] calibration done: %s"
    const std::regex CAL_QGC_FAILED_MSG               ("\\[cal\\] calibration failed: ([\\w\\s:]+)");                            // "[cal] calibration failed: %s"
    const std::regex CAL_QGC_WARNING_MSG              ("\\[cal\\] calibration warning: ([\\w\\s:]+)");                           // "[cal] calibration warning: %s"
    const std::regex CAL_QGC_CANCELLED_MSG            ("\\[cal\\] calibration cancelled");                                      // "[cal] calibration cancelled"
    const std::regex CAL_QGC_PROGRESS_MSG             ("\\[cal\\].* progress <([\\d]+)>");                                 // "[cal] progress <%u>"
    const std::regex CAL_QGC_ORIENTATION_DETECTED_MSG ("\\[cal\\] ([\\w]+) orientation detected");                              // "[cal] %s orientation detected"
    const std::regex CAL_QGC_SIDE_DONE_MSG            ("\\[cal\\] ([\\w]+) side done, rotate to a different side");             // "[cal] %s side done, rotate to a different side"
    const std::regex CAL_ERROR_SENSOR_MSG             ("\\[cal\\] calibration failed: reading sensor");                         // "[cal] calibration failed: reading sensor"
    const std::regex CAL_ERROR_RESET_CAL_MSG          ("\\[cal\\] calibration failed: to reset, sensor ([\\d]+)");              // "[cal] calibration failed: to reset, sensor %u"
    const std::regex CAL_ERROR_APPLY_CAL_MSG          ("\\[cal\\] calibration failed: to apply calibration, sensor ([\\d]+)");  // "[cal] calibration failed: to apply calibration, sensor %u"
    const std::regex CAL_ERROR_SET_PARAMS_MSG         ("\\[cal\\] calibration failed: to set parameters, sensor ([\\d]+)");     // "[cal] calibration failed: to set parameters, sensor %u"
    const std::regex CAL_ERROR_SAVE_PARAMS_MSG        ("\\[cal\\] calibration failed: failed to save parameters");              // "[cal] calibration failed: failed to save parameters"

    const int supportedFwVersion = 2;

    if (!startsWith(text, CAL_PREFIX_MSG))
        return;

    std::smatch match;
    if (std::regex_match(text, match, CAL_QGC_STARTED_MSG))
    {
        BMCL_ASSERT(match.size() == 3);
        if (stoi(match[1]) != 2)
        {
            BMCL_WARNING() << "Unsupported calibration firmware version";
            return;
        }

        //start visual calibration

        std::string sensor = match[2];
        if (sensor == "esc")
        {
            _calibrationStatus.reset();
            _calibrationStatus.message = text;
            return;
        }

        if (!(sensor == "accel" || sensor == "mag" || sensor == "gyro" || sensor == "level") )
        {
            //BMCL_ASSERT(false);
            BMCL_WARNING() << "Unknown sensor: " << sensor;
            return;
        }

        _calibrationStatus.reset();
        _calibrationStatus.message = "Place your vehicle into one of the Incomplete orientations shown below and hold it still";

        if (sensor == "accel")
        {
            _calibrationStatus._sensor = mccmsg::CalibrationSensor::Accelerometer;

            _calibrationStatus.calDownSide.visible = true;
            _calibrationStatus.calUpsideDownSide.visible = true;
            _calibrationStatus.calLeftSide.visible = true;
            _calibrationStatus.calRightSide.visible = true;
            _calibrationStatus.calTailDownSide.visible = true;
            _calibrationStatus.calNoseDownSide.visible = true;
        }
        else if (sensor == "mag")
        {
            _calibrationStatus.calDownSide.visible = true;
            _calibrationStatus.calUpsideDownSide.visible = true;
            _calibrationStatus.calLeftSide.visible = true;
            _calibrationStatus.calRightSide.visible = true;
            _calibrationStatus.calTailDownSide.visible = true;
            _calibrationStatus.calNoseDownSide.visible = true;
//             // Work out what the autopilot is configured to
//             int sides = 0;
//
//             if (_vehicle->parameterManager()->parameterExists(FactSystem::defaultComponentId, "CAL_MAG_SIDES")) {
//                 // Read the requested calibration directions off the system
//                 sides = _vehicle->parameterManager()->getParameter(FactSystem::defaultComponentId, "CAL_MAG_SIDES")->rawValue().toFloat();
//             }
//             else {
//                 // There is no valid setting, default to all six sides
//                 sides = (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2) | (1 << 1) | (1 << 0);
//             }
//
//             _magCalInProgress = true;
//             _calibrationStatus.calTailDownSide.visible = ((sides & (1 << 0)) > 0);
//             _calibrationStatus.calNoseDownSide.visible = ((sides & (1 << 1)) > 0);
//             _calibrationStatus.calLeftSide.visible = ((sides & (1 << 2)) > 0);
//             _calibrationStatus.calRightSide.visible = ((sides & (1 << 3)) > 0);
//             _calibrationStatus.calUpsideDownSide.visible = ((sides & (1 << 4)) > 0);
//             _calibrationStatus.calDownSide.visible = ((sides & (1 << 5)) > 0);
        }
        else if (sensor == "gyro")
        {
            _calibrationStatus._sensor = mccmsg::CalibrationSensor::Gyroscope;
            _calibrationStatus.calDownSide.visible = true;
            _calibrationStatus.calDownSide.inProgress = true;
        }
        else if (sensor == "level")
        {
            _calibrationStatus._sensor = mccmsg::CalibrationSensor::Level;
            _calibrationStatus.calDownSide.visible = true;
            _calibrationStatus.calDownSide.inProgress = true;
        }
//            emit orientationCalSidesDoneChanged();
//            emit orientationCalSidesVisibleChanged();
//            emit orientationCalSidesInProgressChanged();
//            _updateAndEmitShowOrientationCalArea(true);
    }
    else if (std::regex_match(text, match, CAL_QGC_DONE_MSG))
    {
        _calibrationStatus.calDownSide.done = true;
        _calibrationStatus.calUpsideDownSide.done = true;
        _calibrationStatus.calLeftSide.done = true;
        _calibrationStatus.calRightSide.done = true;
        _calibrationStatus.calNoseDownSide.done = true;
        _calibrationStatus.calTailDownSide.done = true;

        _calibrationStatus.progress = 100;
        _calibrationStatus.done = true;
        _calibrationStatus.message = "Complete";
        _calibrationStarted = false;
    }
    else if (std::regex_match(text, match, CAL_QGC_FAILED_MSG))
    {
        _calibrationStarted = false;
        _calibrationStatus.progress = 0;
        _calibrationStatus.failed = true;
        _calibrationStatus.done = false;
    }
    else if (std::regex_match(text, match, CAL_QGC_CANCELLED_MSG))
    {
        _calibrationStarted = false;
        _calibrationStatus.progress = 0;
        _calibrationStatus.failed = true;
        _calibrationStatus.done = false;
    }
    else if (std::regex_match(text, match, CAL_QGC_PROGRESS_MSG))
    {
        BMCL_ASSERT(match.size() == 2);
        int progress = stoi(match[1]);
        if (progress > 100)
            progress = 100;
        _calibrationStatus.progress = progress;
    }
    else if (std::regex_match(text, match, CAL_QGC_ORIENTATION_DETECTED_MSG))
    {
        BMCL_ASSERT(match.size() == 2);
        std::string side = match[1];
        BMCL_DEBUG() << "Side started: " << side;

        if (side == "down")
        {
            _calibrationStatus.calDownSide.inProgress = true;
            _calibrationStatus.calDownSide.rotate = _calibrationStatus.isMagnetometer();
        }
        else if (side == "up")
        {
            _calibrationStatus.calUpsideDownSide.inProgress = true;
            _calibrationStatus.calUpsideDownSide.rotate = _calibrationStatus.isMagnetometer();
        }
        else if (side == "left")
        {
            _calibrationStatus.calLeftSide.inProgress = true;
            _calibrationStatus.calLeftSide.rotate = _calibrationStatus.isMagnetometer();
        }
        else if (side == "right")
        {
            _calibrationStatus.calRightSide.inProgress = true;
            _calibrationStatus.calRightSide.rotate = _calibrationStatus.isMagnetometer();
        }
        else if (side == "front")
        {
            _calibrationStatus.calNoseDownSide.inProgress = true;
            _calibrationStatus.calNoseDownSide.rotate = _calibrationStatus.isMagnetometer();
        }
        else if (side == "back") {
            _calibrationStatus.calTailDownSide.inProgress = true;
            _calibrationStatus.calTailDownSide.rotate = _calibrationStatus.isMagnetometer();
        }

        if(_calibrationStatus.isMagnetometer())
        {
            _calibrationStatus.message = "Rotate the vehicle continuously as shown in the diagram until marked as Completed";
        }
        else
        {
            _calibrationStatus.message = "Hold still in the current orientation";
        }
    }
    else if (std::regex_match(text, match, CAL_QGC_SIDE_DONE_MSG))
    {
        BMCL_ASSERT(match.size() == 2);
        std::string side = match[1];
        BMCL_DEBUG() << "Side finished: " << side;

        if (side == "down")
        {
            _calibrationStatus.calDownSide.inProgress = false;
            _calibrationStatus.calDownSide.done = true;
            _calibrationStatus.calDownSide.rotate = false;
        }
        else if (side == "up")
        {
            _calibrationStatus.calUpsideDownSide.inProgress = false;
            _calibrationStatus.calUpsideDownSide.done = true;
            _calibrationStatus.calUpsideDownSide.rotate = false;
        }
        else if (side == "left")
        {
            _calibrationStatus.calLeftSide.inProgress = false;
            _calibrationStatus.calLeftSide.done = true;
            _calibrationStatus.calLeftSide.rotate = false;
        }
        else if (side == "right")
        {
            _calibrationStatus.calRightSide.inProgress = false;
            _calibrationStatus.calRightSide.done = true;
            _calibrationStatus.calRightSide.rotate = false;
        }
        else if (side == "front")
        {
            _calibrationStatus.calNoseDownSide.inProgress = false;
            _calibrationStatus.calNoseDownSide.done = true;
            _calibrationStatus.calNoseDownSide.rotate = false;
        }
        else if (side == "back")
        {
            _calibrationStatus.calTailDownSide.inProgress = false;
            _calibrationStatus.calTailDownSide.done = true;
            _calibrationStatus.calTailDownSide.rotate = false;
        }

        _calibrationStatus.message = "Place you vehicle into one of the orientations shown below and hold it still";
    }
    else if (std::regex_match(text, match, CAL_QGC_WARNING_MSG))
    {

    }
    else if (std::regex_match(text, match, CAL_ERROR_SENSOR_MSG))
    {

    }
    else if (std::regex_match(text, match, CAL_ERROR_RESET_CAL_MSG))
    {

    }
    else if (std::regex_match(text, match, CAL_ERROR_APPLY_CAL_MSG))
    {

    }
    else if (std::regex_match(text, match, CAL_ERROR_SET_PARAMS_MSG))
    {

    }
    else if (std::regex_match(text, match, CAL_ERROR_SAVE_PARAMS_MSG))
    {

    }
    else
    {
        //BMCL_CRITICAL() << "failed to parse [cal] msg";
    }

    if (_calibrationStatus._sensor == mccmsg::CalibrationSensor::Esc)
    {
        _calibrationStatus.message = text;
    }

    _helper.sendTrait(new mccmsg::TmCalibration(_id.device(), _calibrationStatus));
}

void TraitSensorCalibration::extractIntFromString(const std::string& text, const std::string& start, const std::string& stop) const
{

}
}