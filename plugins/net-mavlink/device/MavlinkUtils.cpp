#include <caf/event_based_actor.hpp>

#include <bmcl/Panic.h>
#include <bmcl/Assert.h>
#include <bmcl/MemWriter.h>
#include <bmcl/Logging.h>

#include "mcc/msg/NetVariant.h"
#include "mcc/net/NetLoggerInf.h"

#include "../device/Mavlink.h"
#include "../device/MavlinkUtils.h"

namespace mccmav {

const char * ackTypeToString(uint8_t type)
{
    switch (type)
    {
    case 0:
        return "Success";
    case MAV_MISSION_UNSUPPORTED_FRAME:
        return "Coordinate frame unsupported";
    case MAV_MISSION_UNSUPPORTED:
        return "Unsupported command";
    case MAV_MISSION_NO_SPACE:
        return "Mission count exceeds storage";
    case MAV_MISSION_INVALID:
        return "A specified parameter was invalid (parameter not set)";
    case MAV_MISSION_INVALID_PARAM1:
        return "A specified parameter was invalid (param1)";
    case MAV_MISSION_INVALID_PARAM2:
        return "A specified parameter was invalid (param2)";
    case MAV_MISSION_INVALID_PARAM3:
        return "A specified parameter was invalid (param3)";
    case MAV_MISSION_INVALID_PARAM4:
        return "A specified parameter was invalid (param4)";
    case MAV_MISSION_INVALID_PARAM5_X:
        return "A specified parameter was invalid (param5/x)";
    case MAV_MISSION_INVALID_PARAM6_Y:
        return "A specified parameter was invalid (param6/y)";
    case MAV_MISSION_INVALID_PARAM7:
        return "A specified parameter was invalid (param7/z)";
    case MAV_MISSION_INVALID_SEQUENCE:
        return "Mission received out of sequence";
    case MAV_MISSION_DENIED:
        return "UAS not accepting missions";
    case MAV_MISSION_ERROR:
    default:
        return "Unspecified error";
    }
}

const char * frameToString(MAV_FRAME frame)
{
    switch (frame)
    {
    case MAV_FRAME_GLOBAL:
        return "Global coordinate frame, WGS84 coordinate system. First value / x: latitude, second value / y: longitude, third value / z: positive altitude over mean sea level (MSL)";
    case MAV_FRAME_LOCAL_NED:
        return "Local coordinate frame, Z-up (x: north, y: east, z: down).";
    case MAV_FRAME_MISSION:
        return "NOT a coordinate frame, indicates a mission command.";
    case MAV_FRAME_GLOBAL_RELATIVE_ALT:
        return "Global coordinate frame, WGS84 coordinate system, relative altitude over ground with respect to the home position. First value / x: latitude, second value / y: longitude, third value / z: positive altitude with 0 being at the altitude of the home location.";
    case MAV_FRAME_LOCAL_ENU:
        return "Local coordinate frame, Z-down (x: east, y: north, z: up)";
    case MAV_FRAME_GLOBAL_INT:
        return "Global coordinate frame, WGS84 coordinate system. First value / x: latitude in degrees*1.0e-7, second value / y: longitude in degrees*1.0e-7, third value / z: positive altitude over mean sea level (MSL)";
    case MAV_FRAME_GLOBAL_RELATIVE_ALT_INT:
        return "Global coordinate frame, WGS84 coordinate system, relative altitude over ground with respect to the home position. First value / x: latitude in degrees*10e-7, second value / y: longitude in degrees*10e-7, third value / z: positive altitude with 0 being at the altitude of the home location.";
    case MAV_FRAME_LOCAL_OFFSET_NED:
        return "Offset to the current local frame. Anything expressed in this frame should be added to the current local frame position.";
    case MAV_FRAME_BODY_NED:
        return "Setpoint in body NED frame. This makes sense if all position control is externalized - e.g. useful to command 2 m/s^2 acceleration to the right.";
    case MAV_FRAME_BODY_OFFSET_NED:
        return "Offset in body NED frame. This makes sense if adding setpoints to the current flight path, to avoid an obstacle - e.g. useful to command 2 m/s^2 acceleration to the east.";
    case MAV_FRAME_GLOBAL_TERRAIN_ALT:
        return "Global coordinate frame with above terrain level altitude. WGS84 coordinate system, relative altitude over terrain with respect to the waypoint coordinate. First value / x: latitude in degrees, second value / y: longitude in degrees, third value / z: positive altitude in meters with 0 being at ground level in terrain model.";
    case MAV_FRAME_GLOBAL_TERRAIN_ALT_INT:
        return "Global coordinate frame with above terrain level altitude. WGS84 coordinate system, relative altitude over terrain with respect to the waypoint coordinate. First value / x: latitude in degrees*10e-7, second value / y: longitude in degrees*10e-7, third value / z: positive altitude in meters with 0 being at ground level in terrain model.";
    default:
        bmcl::panic("not implemented");
    }
}

const char * modeToString(MAV_MODE mode)
{
    switch (mode)
    {
    case MAV_MODE_PREFLIGHT:
        return "MAV_MODE_PREFLIGHT System is not ready to fly, booting, calibrating, etc. No flag is set.";
    case MAV_MODE_MANUAL_DISARMED:
        return "MAV_MODE_MANUAL_DISARMED System is allowed to be active, under manual (RC) control, no stabilization";
    case MAV_MODE_TEST_DISARMED:
        return "MAV_MODE_TEST_DISARMED UNDEFINED mode. This solely depends on the autopilot - use with caution, intended for developers only.";
    case MAV_MODE_STABILIZE_DISARMED:
        return "MAV_MODE_STABILIZE_DISARMED System is allowed to be active, under assisted RC control.";
    case MAV_MODE_GUIDED_DISARMED:
        return "MAV_MODE_GUIDED_DISARMED System is allowed to be active, under autonomous control, manual setpoint";
    case MAV_MODE_AUTO_DISARMED:
        return "MAV_MODE_AUTO_DISARMED System is allowed to be active, under autonomous control and navigation (the trajectory is decided onboard and not pre-programmed by MISSIONs)";
    case MAV_MODE_MANUAL_ARMED:
        return "MAV_MODE_MANUAL_ARMED System is allowed to be active, under manual (RC) control, no stabilization";
    case MAV_MODE_TEST_ARMED:
        return "MAV_MODE_TEST_ARMED UNDEFINED mode. This solely depends on the autopilot - use with caution, intended for developers only.";
    case MAV_MODE_STABILIZE_ARMED:
        return "MAV_MODE_STABILIZE_ARMED System is allowed to be active, under assisted RC control.";
    case MAV_MODE_GUIDED_ARMED:
        return "MAV_MODE_GUIDED_ARMED System is allowed to be active, under autonomous control, manual setpoint";
    case MAV_MODE_AUTO_ARMED:
        return "MAV_MODE_AUTO_ARMED System is allowed to be active, under autonomous control and navigation (the trajectory is decided onboard and not pre-programmed by MISSIONs)";
    default:
        bmcl::panic("not implemented");
    }
}

const char * resultToString(MAV_RESULT result)
{
    switch (result)
    {
    case MAV_RESULT_ACCEPTED:
        return "Command ACCEPTED and EXECUTED";
    case MAV_RESULT_TEMPORARILY_REJECTED:
        return "Command TEMPORARY REJECTED/DENIED";
    case MAV_RESULT_DENIED:
        return "Command PERMANENTLY DENIED";
    case MAV_RESULT_UNSUPPORTED:
        return "Command UNKNOWN/UNSUPPORTED";
    case MAV_RESULT_FAILED:
        return "Command executed, but failed";
    default:
        bmcl::panic("not implemented");
    }
}

mccmsg::NetVariant toNetVariant(const mavlink_param_union_t& paramUnion)
{
    switch (paramUnion.type) {
    case MAV_PARAM_TYPE_REAL32:
        return (double)paramUnion.param_float;
    case MAV_PARAM_TYPE_UINT8:
        return (uint8_t)paramUnion.param_uint8;
    case MAV_PARAM_TYPE_INT8:
        return (int8_t)paramUnion.param_int8;
    case MAV_PARAM_TYPE_UINT16:
        return (uint16_t)paramUnion.param_uint16;
    case MAV_PARAM_TYPE_INT16:
        return (int16_t)paramUnion.param_int16;
    case MAV_PARAM_TYPE_UINT32:
        return (uint32_t)paramUnion.param_uint32;
    case MAV_PARAM_TYPE_INT32:
        return (int32_t)paramUnion.param_int32;
    default:
        BMCL_CRITICAL() << "INVALID DATA TYPE USED AS PARAMETER VALUE: " << paramUnion.type;
    }
    return mccmsg::NetVariant();
}

MavlinkMessageRc::MavlinkMessageRc(const mavlink_message_t& m)
{
    *(mavlink_message_t*)this = m;
}
MavlinkMessageRc::MavlinkMessageRc() {}
MavlinkMessageRc::~MavlinkMessageRc(){}

}
