#pragma once
#include <caf/atom.hpp>
#include <caf/event_based_actor.hpp>

#include <bmcl/Option.h>

#include "mcc/uav/Structs.h"

#include "mcc/msg/ptr/Fwd.h"
#include "mcc/msg/Route.h"

#include "mcc/net/Cmd.h"
#include "mcc/net/TmHelper.h"
#include "../device/MavlinkUtils.h"
#include "../traits/Trait.h"
#include "../device/Route.h"

namespace mccmav {

class TraitRoutes : public caf::event_based_actor
{
    friend class RouteVisitor;
public:
    TraitRoutes(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const mccmsg::ProtocolId& id, const std::string& name);
    caf::behavior make_behavior() override;
    const char* name() const override;

private:
    enum class WaypointsState
    {
        Outdated,
        ReadingCount,
        Reading,
        Ok,
        WritingBuffer,
        Clearing,
        Writing,
        WritingAck
    };

    void handleTmMessage(const MavlinkMessagePtr& message);
    void provideTm();

    void execute(mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteSetPtr& msg);
    void execute(mccnet::CmdPtr&&, const mccmsg::CmdRouteSetActivePoint&);

    // Read MAV Waypoint list
    void requestWaypointList();
    void handleMissionCountMessage(const MavlinkMessagePtr &message);

    void requestNextWaypoint(uint16_t seq);
    void handleMissionItemMessage(const MavlinkMessagePtr &message);

    void sendWaypointAck(MAV_MISSION_RESULT result);
    // Read MAV Waypoint list

    // Write MAV Waypoint list
    void sendWaypointCount(uint16_t count, const mccnet::CmdPtr& cmd);
    void sendWaypoint(MAV_CMD mavCmd, const mccnet::CmdPtr& cmd);
    void handleMissionAckMessage(const MavlinkMessagePtr &message, const mccnet::CmdPtr& cmd);
    void handleWaypointRequestMessage(const MavlinkMessagePtr& message, const mccnet::CmdPtr& cmd);

    void sendActiveWaypoint(uint16_t point);

    void toState(WaypointsState newState);

    static const char * stateToString(WaypointsState state);

private:
    bool _isActive;

    std::string _name;
    caf::actor _core;
    caf::actor _broker;
    mccnet::TmHelper _helper;

    mccmsg::Device _device;

    uint8_t _targetSystem;
    uint8_t _targetComponent;

    WaypointsState _waypointsState;
    uint_fast16_t _waypointsCount;
    uint16_t _waypointIndex;
    Route _route;

    bmcl::Option<mccmsg::Route> _routeSet;

    uint_fast8_t _activeRoute;
    uint16_t _crc;

    bmcl::Option<mccmsg::CmdParamList> _cmd;
    bool _missionExecuting;
};
}