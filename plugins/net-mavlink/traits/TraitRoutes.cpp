#include <utility>
#include <bmcl/Logging.h>
#include <bmcl/MemWriter.h>
#include <bmcl/MakeRc.h>
#include <fmt/format.h>

#include "mcc/msg/Route.h"
#include "mcc/net/NetLoggerInf.h"
#include "../device/MavlinkUtils.h"
#include "../device/Mavlink.h"
#include "Firmware.h"
#include "../traits/TraitRoutes.h"

#include "mcc/uav/Structs.h"


CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmav::MavlinkMessagePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NotificationPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::CmdRouteSetPtr);

namespace mccmav {


TraitRoutes::TraitRoutes(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const mccmsg::ProtocolId& id, const std::string& name)
    : caf::event_based_actor(cfg)
    , _isActive(false)
    , _helper(id, core, this)
    , _broker(broker)
    , _name(name)
    , _device(id.device())
    , _targetSystem((uint8_t)id.id())
    , _targetComponent(0)
    , _waypointsState(WaypointsState::Outdated)
    , _waypointsCount(0)
    , _activeRoute(0)
    , _crc(0)
    , _cmd(bmcl::None)
    , _missionExecuting(false)
{
}

const char* TraitRoutes::name() const
{
    return _name.c_str();
}

void TraitRoutes::requestWaypointList()
{
    MavlinkMessageRc* msg = new MavlinkMessageRc;
    mavlink_msg_mission_request_list_pack_chan(_targetSystem, _targetComponent, 0, msg, _targetSystem, _targetComponent, MAV_MISSION_TYPE::MAV_MISSION_TYPE_MISSION);

    request(_broker, std::chrono::seconds(3), waypoint_msg_atom::value, MavlinkMessagePtr(msg), MAVLINK_MSG_ID_MISSION_COUNT).then(
        [this](const MavlinkMessagePtr& message)
        {
            handleMissionCountMessage(message);
        }
      , [this](const caf::error& err)
        {
            BMCL_DEBUG() << "requestWaypointList: " << system().render(err);
            toState(WaypointsState::Outdated);
        }
    );

    toState(WaypointsState::ReadingCount);
}

void TraitRoutes::requestNextWaypoint(uint16_t seq)
{
    MavlinkMessageRc* msg = new MavlinkMessageRc;
    mavlink_msg_mission_request_pack_chan(_targetSystem, _targetComponent, 0, msg, _targetSystem, _targetComponent, seq, MAV_MISSION_TYPE::MAV_MISSION_TYPE_MISSION);

    request(_broker, std::chrono::seconds(3), waypoint_msg_atom::value, MavlinkMessagePtr(msg), MAVLINK_MSG_ID_MISSION_ITEM).then(
        [this](const MavlinkMessagePtr& message)
        {
            handleMissionItemMessage(message);
        }
      , [this](const caf::error& err)
        {
            BMCL_DEBUG() << "requestNextWaypoint: " << system().render(err);
            toState(WaypointsState::Outdated);
        }
    );
}

void TraitRoutes::handleTmMessage(const MavlinkMessagePtr& message)
{
    switch (message->msgid)
    {
    case MAVLINK_MSG_ID_MISSION_ITEM_REACHED:
    {
        mavlink_mission_item_reached_t itemReached;
        mavlink_msg_mission_item_reached_decode(message.get(), &itemReached);
        BMCL_DEBUG() << "Received waypoint reached";
        uint16_t seq = itemReached.seq;
        if (seq > 0)
            _route.setNextWaypoint(static_cast<uint16_t>(seq));
    }
    break;
    case MAVLINK_MSG_ID_MISSION_CURRENT:
    {
        mavlink_mission_current_t current;
        mavlink_msg_mission_current_decode(message.get(), &current);

        uint8_t seq = current.seq;

        _route.setNextWaypoint(static_cast<uint16_t>(seq));
    }
   break;
    default:
        break;
    }
}



void TraitRoutes::sendWaypointAck(MAV_MISSION_RESULT result)
{
    MavlinkMessageRc* msg = new MavlinkMessageRc;
    mavlink_msg_mission_ack_pack_chan(_targetSystem, _targetComponent, 0, msg, _targetSystem, _targetComponent, result, MAV_MISSION_TYPE::MAV_MISSION_TYPE_MISSION);

    request(_broker, caf::infinite, waypoint_msg_no_rep_atom::value, MavlinkMessagePtr(msg)).then(
        [this]()
        {
        }
      , [this](const caf::error& err)
        {
            BMCL_DEBUG() << "sendWaypointAck: " << system().render(err);
            toState(WaypointsState::Outdated);
        }
    );

    toState(WaypointsState::Ok);
}

void TraitRoutes::provideTm()
{
    if (_waypointsState != WaypointsState::Ok)
        return;

    bmcl::Option<std::size_t> guided;

    mccmsg::RouteProperties propGuided(1, 0, "Точка GUIDED режима", mccmsg::BitFlags(mccmsg::RouteFlag::PointsOnly), guided);
    mccmsg::RouteProperties propMain(100, 1, "Основной маршрут", _route.nextWaypoint());

    {
        mccmsg::RoutesProperties list = { propGuided, propMain };
        _helper.sendTrait(new mccmsg::TmRoutesList(_device, std::move(list), _missionExecuting ? propMain.name : propGuided.name));
    }

    {
        mccmsg::Waypoints points;
        for (const auto& i : _route.points())
        {
            mccmsg::Properties properties;
//             switch (i.command)
//             {
//             case MAV_CMD_NAV_TAKEOFF:
//                 properties.set(mccmsg::WaypointProperties::Home, true);
//                 break;
//             case MAV_CMD_NAV_LAND:
//                 properties.set(mccmsg::WaypointProperties::Landing, true);
//                 break;
//             }
// 
//             auto x = properties.to_uint();

//            points.push_back(mccmsg::Waypoint(0, 0, properties));
        }
        _helper.sendTrait(new mccmsg::TmRoute(_device, mccmsg::Route(std::move(points), std::move(propMain))));
    }
}

void TraitRoutes::execute(mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteSetPtr& msg)
{
    auto route = msg->route();
    auto routeName = route.properties.name;
    if (routeName != 1)
    {
        cmd->sendFailed("Поддерживаются только маршруты с именем 1");
        return;
    }
    _routeSet = msg->route();

    sendWaypointCount((uint16_t)_routeSet->waypoints.size(), cmd);
}

void TraitRoutes::execute(mccnet::CmdPtr&& rp, const mccmsg::CmdRouteSetActivePoint& cmd)
{
    auto point = cmd.point();

    if (point.isSome() && _route.size() <= *point)
    {
        rp->sendFailed("Недопустимый индекс активной точки в маршруте");
        return;
    }

    _route.setNextWaypoint(point.isNone() ? 0 : *cmd.point());
    sendActiveWaypoint(point.isNone() ? 0 : *cmd.point());
}

void TraitRoutes::toState(TraitRoutes::WaypointsState newState)
{
    BMCL_DEBUG() << stateToString(_waypointsState) << " -> " << stateToString(newState);
    _waypointsState = newState;
}

const char * TraitRoutes::stateToString(WaypointsState state)
{
    switch (state)
    {
    case WaypointsState::Outdated:
        return "Outdated";
    case WaypointsState::ReadingCount:
        return "ReadingCount";
    case WaypointsState::Reading:
        return "Reading";
    case WaypointsState::Ok:
        return "Ok";
    case WaypointsState::WritingBuffer:
        return "WritingBuffer";
    case WaypointsState::Writing:
        return "Writing";
    case WaypointsState::Clearing:
        return "Clearing";
    case WaypointsState::WritingAck:
        return "WritingAck";
    }

    return "Unknown";
}

caf::behavior TraitRoutes::make_behavior()
{
    using timer_atom = caf::atom_constant<caf::atom("timer")>;
    send(this, timer_atom::value);

    return
    {
        [this](activated_atom)
        {
            _isActive = true;
        }
      , [this](deactivated_atom)
        {
            _isActive = false;
            toState(WaypointsState::Outdated);
        }
      , [this](timer_atom)
        {
            provideTm();
            if (_waypointsState == WaypointsState::Outdated && _isActive)
                requestWaypointList();

            delayed_send(this, std::chrono::milliseconds(100), timer_atom::value);
        }
      ,[this](frmupdate_atom, const FirmwarePtr& frm)
        {

        }
      , [this](const mccmsg::CmdRouteSetPtr& cmd)
        {
            execute(bmcl::makeRc<mccnet::Cmd>(make_response_promise(), cmd), cmd);
        }
      , [this](const MavlinkMessagePtr& tm)
        {
            handleTmMessage(tm);
        }
      , [this](mission_state_atom, bool state)
        {
            _missionExecuting = state;
        }
    };
}

void TraitRoutes::handleMissionCountMessage(const MavlinkMessagePtr &message)
{
    BMCL_ASSERT(message->msgid == MAVLINK_MSG_ID_MISSION_COUNT);
    BMCL_ASSERT(_waypointsState == WaypointsState::ReadingCount);

    mavlink_mission_count_t missionCount;
    mavlink_msg_mission_count_decode(message.get(), &missionCount);

    _route.clear();
    _waypointsCount = missionCount.count;
    BMCL_DEBUG() << "Waypoints count received: " << _waypointsCount;
    _route.reserve(_waypointsCount);
    if (_waypointsCount > 0)
    {
        toState(WaypointsState::Reading);
        BMCL_DEBUG() << "Waypoints" << _waypointsCount;
        requestNextWaypoint(0);
    }
    else
    {
        BMCL_DEBUG() << "No waypoints";
        sendWaypointAck(MAV_MISSION_RESULT::MAV_MISSION_ACCEPTED);
        toState(WaypointsState::Ok);
    }
}

void TraitRoutes::handleMissionItemMessage(const MavlinkMessagePtr &message)
{
    BMCL_ASSERT(message->msgid == MAVLINK_MSG_ID_MISSION_ITEM);
    BMCL_ASSERT(_waypointsState == WaypointsState::Reading);

    mavlink_mission_item_t missionItem;
    mavlink_msg_mission_item_decode(message.get(), &missionItem);

    uint8_t frame = missionItem.frame;
    uint16_t seq = missionItem.seq;

    BMCL_DEBUG() << "Received waypoint seq: " << seq;
    float latitude = missionItem.x;
    float longitude = missionItem.y;
    float altitude = missionItem.z;

    BMCL_DEBUG() << fmt::format("Waypoint received (lat: {}, lon: {}, alt: {}, frame: {}",
                                QString::number(latitude, 'f', 12).toStdString(), QString::number(longitude, 'f', 12).toStdString(),
                                QString::number(altitude, 'f', 12).toStdString(), frameToString((MAV_FRAME)frame));

    bool isCurrent = (missionItem.current != 0);

    uint16_t command = missionItem.command;

    MavlinkPoint point(latitude, longitude, altitude, command);
    auto index = seq;
    if (index == _route.size())
    {
        _route.push_back(std::move(point));

        if (isCurrent)
        {
            _activeRoute = 1;
            _route.setNextWaypoint(_route.size() - 1);
        }
    }
    else
    {
        BMCL_WARNING() << "Received waypoint with invalid seq";
    }

    if (_waypointsCount == _route.size())
    {
        BMCL_DEBUG() << "Mavlink route downloaded successfully";
        sendWaypointAck(MAV_MISSION_RESULT::MAV_MISSION_ACCEPTED);
        toState(WaypointsState::Ok);
    }
    else
    {
        requestNextWaypoint((uint16_t)_route.size());
    }
}

void TraitRoutes::sendWaypointCount(uint16_t count, const mccnet::CmdPtr& cmd)
{
    BMCL_ASSERT(_waypointsState == WaypointsState::Ok);

    MavlinkMessageRc* msg = new MavlinkMessageRc;
    mavlink_msg_mission_count_pack_chan(_targetSystem, _targetComponent, 0, msg, _targetSystem, _targetComponent, count, MAV_MISSION_TYPE::MAV_MISSION_TYPE_MISSION);

    request(_broker, std::chrono::seconds(3), waypoint_msg_atom::value, MavlinkMessagePtr(msg), MAVLINK_MSG_ID_MISSION_REQUEST).then(
        [this, cmd](const MavlinkMessagePtr& message)
        {
            handleWaypointRequestMessage(message, cmd);
        }
      , [this](const caf::error& err)
        {
            BMCL_DEBUG() << "sendWaypointCount: " << system().render(err);
            toState(WaypointsState::Outdated);
        }
    );

    _waypointIndex = 0;
    toState(WaypointsState::Writing);
}

void TraitRoutes::handleWaypointRequestMessage(const MavlinkMessagePtr& message, const mccnet::CmdPtr& cmd)
{
    BMCL_ASSERT(message->msgid == MAVLINK_MSG_ID_MISSION_REQUEST);
    BMCL_ASSERT(_waypointsState == WaypointsState::Writing);

    mavlink_mission_request_t request;
    mavlink_msg_mission_request_decode(message.get(), &request);

    if (_waypointIndex != request.seq)
    {
        BMCL_WARNING() << "_waypointIndex != request.seq " <<  _waypointIndex << request.seq;
        return;
    }

    if (_routeSet.isNone() || request.seq >= _routeSet->waypoints.size())
    {
        BMCL_WARNING() << "request.seq >= _routeBuffer.size()";
        return;
    }

    if (_waypointIndex == 0) // Первая точка -- точка взлета
    {
        sendWaypoint(MAV_CMD_NAV_TAKEOFF, cmd);
    }
    else
    {
        sendWaypoint(MAV_CMD_NAV_WAYPOINT, cmd);
    }
}

void TraitRoutes::sendWaypoint(MAV_CMD mavCmd, const mccnet::CmdPtr& cmd)
{
    if (_routeSet.isNone())
    {
        toState(WaypointsState::Outdated);
        return;
    }

    auto wp = _routeSet->waypoints.at(_waypointIndex);
    BMCL_DEBUG() << "RouteController::sendWaypoint " << _waypointIndex;
    MavlinkMessageRc* msg = new MavlinkMessageRc;
    mavlink_msg_mission_item_pack_chan(_targetSystem, _targetComponent, 0, msg, _targetSystem, _targetComponent, _waypointIndex, MAV_FRAME_GLOBAL, mavCmd,
                                       _waypointIndex == 0 ? 1 : 0, false, 0.0f, 0.0f, 0.0f, 0.0f, wp.position.latitude(), wp.position.longitude(), wp.position.altitude()
                                       , MAV_MISSION_TYPE::MAV_MISSION_TYPE_MISSION);

    if (_waypointIndex != _routeSet->waypoints.size() - 1)
    {
        request(_broker, std::chrono::seconds(3), waypoint_msg_atom::value, MavlinkMessagePtr(msg), MAVLINK_MSG_ID_MISSION_REQUEST).then(
        [this, cmd](const MavlinkMessagePtr& message)
        {
            handleWaypointRequestMessage(message, cmd);
        }
      , [this](const caf::error& err)
        {
            BMCL_DEBUG() << "sendWaypoint: " << system().render(err);
            toState(WaypointsState::Outdated);
        }
        );
        ++_waypointIndex;
    }
    else
    {
        request(_broker, std::chrono::seconds(3), waypoint_msg_atom::value, MavlinkMessagePtr(msg), MAVLINK_MSG_ID_MISSION_ACK).then(
            [this, cmd](const MavlinkMessagePtr& message)
        {
            handleMissionAckMessage(message, cmd);
        }
       , [this](const caf::error& err)
        {
            BMCL_DEBUG() << "sendWaypoint wait ack: " << system().render(err);
            toState(WaypointsState::Outdated);
        }
        );
    }

    uint8_t progress = 100 * _waypointIndex / (uint16_t)_routeSet->waypoints.size();
    if (_waypointIndex == _routeSet->waypoints.size() - 1)
        progress = 100;

    BMCL_DEBUG() << "PROGRESS " << progress;
    cmd->sendProgress(progress);
}

void TraitRoutes::handleMissionAckMessage(const MavlinkMessagePtr &message, const mccnet::CmdPtr& cmd)
{
    if (_routeSet.isNone())
    {
        BMCL_WARNING() << "Mission sent error!";
        cmd->sendFailed("Mission sent error!");
        toState(WaypointsState::Outdated);
        return;
    }

    mavlink_mission_ack_t ack;
    mavlink_msg_mission_ack_decode(message.get(), &ack);

    if (_waypointsState == WaypointsState::Writing)
    {
        switch (ack.type)
        {
        case MAV_MISSION_RESULT::MAV_MISSION_ACCEPTED:
            BMCL_DEBUG() << "Mission sent success";
            for (size_t i = 0; i < _routeSet->waypoints.size(); ++i)
            {
//                 auto waypoint = _routeSet.waypoints.at(i);
//                 uint16_t flags = waypoint.properties.to_uint();

//                 MAN_FLYING_MODE mode = MAN_FLYING_MODE_ENUM_END;
//                 if (flags & (uint16_t)mccuav::WaypointType::Reynolds)
//                     mode = MAN_FLYING_MODE_REYNOLDS;
//                 else if (flags & (uint16_t)mccuav::WaypointType::Formation)
//                     mode = MAN_FLYING_MODE_FORMATION;
//                 else if (flags & (uint16_t)mccuav::WaypointType::Snake)
//                     mode = MAN_FLYING_MODE_SNAKE;
//                 else if (flags & (uint16_t)mccuav::WaypointType::Loop)
//                     mode = MAN_FLYING_MODE_LOOP;
//
//                 if (mode != MAN_FLYING_MODE_ENUM_END)
//                 {
//                     MavlinkMessageRc* msg = new MavlinkMessageRc;
//                     mavlink_msg_man_set_waypoint_flying_mode_pack_chan(_targetSystem, _targetComponent, 0, msg, i, mode);
//
//                        request(_broker, caf::infinite, waypoint_msg_no_rep_atom::value, MavlinkMesssagePtr(msg)).then(
//                            [this]()
//                        {
//                        }
//                      , [this](const caf::error& err)
//                        {
//                        }
//                     );
//                 }
            }
            cmd->sendDone();
            toState(WaypointsState::Outdated);
            break;
        case MAV_MISSION_RESULT::MAV_MISSION_ERROR:
            BMCL_WARNING() << "Mission sent error!";
            cmd->sendFailed("Mission sent error!");
            toState(WaypointsState::Outdated);
            break;
        default:
            BMCL_WARNING() << "Mission ack: " << ack.type;
            break;
        }
    }
}

void TraitRoutes::sendActiveWaypoint(uint16_t point)
{
    MavlinkMessageRc* msg = new MavlinkMessageRc;
    mavlink_msg_mission_set_current_pack_chan(_targetSystem, _targetComponent, 0, msg, _targetSystem, _targetComponent, point);

    request(_broker, caf::infinite, waypoint_msg_no_rep_atom::value, MavlinkMessagePtr(msg)).then(
           [this]()
       {
       }
     , [this](const caf::error& err)
       {
       }
    );
}
}