#include <chrono>
#include <algorithm>

#include <QByteArray>

#include <bmcl/Logging.h>
#include <bmcl/OptionPtr.h>
#include <bmcl/Math.h>
#include <bmcl/Reader.h>
#include <bmcl/MakeRc.h>
#include <fmt/format.h>

#include "mcc/msg/GroupState.h"
#include "mcc/msg/exts/Gps.h"

#include "mcc/msg/Cmd.h"
#include "mcc/msg/FwdExt.h"
#include "mcc/msg/ptr/Device.h"
#include "mcc/net/NetLoggerInf.h"
#include "mcc/net/Group.h"

#include "../device/MavlinkUtils.h"
#include "../device/Mavlink.h"
#include "../device/Device.h"
#include "../device/Tm.h"
#include "../device/Px4Fsm.h"

#include "../traits/TraitRoutes.h"
#include "../traits/TraitRegistrator.h"
#include "../traits/TraitParams.h"
#include "../traits/TraitSensorCalibration.h"
#include "../traits/TraitRadioCalibration.h"
#include "../traits/TraitJoystick.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Device);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NetVariant);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::ProtocolId);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::PacketPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmav::MavlinkMessagePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mavlink_heartbeat_t);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mavlink_rc_channels_t);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mavlink_param_value_t);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::vector<mccmav::ParamValue>);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmav::FirmwarePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccnet::GroupState);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccnet::GroupDeviceState);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::CmdCalibrationCancelPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::CmdCalibrationStartPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::CmdRouteSetPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::CmdParamReadPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::CmdParamWritePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NotificationPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::DevReqPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::device::Activate_ResponsePtr)

extern mavlink_status_t m_mavlink_status[MAVLINK_COMM_NUM_BUFFERS];

namespace mccmav {

template<typename T, typename... A>
caf::actor Device::createTrait(const char* s, A&&... args)
{
    caf::actor a = spawn<T, caf::linked>(_core, this, _id, name() + std::string(s), std::forward<A>(args)...);
    _traits.push_back(a);
    return a;
}

void Device::sendMavlinkMessageToChannel(const MavlinkMessagePtr& message)
{
    mccmsg::PacketPtr bytes = new mccmsg::Packet();
    bytes->resize(MAVLINK_MAX_PACKET_LEN);

    int lenght = mavlink_msg_to_send_buffer(bytes->data(), message.get());

    if (!lenght) return;
    bytes->resize(lenght);

    _stats._sent.add(lenght, 1);

    send(_broker, mccnet::req_atom::value, _device, std::move(bytes));
}

template<typename T>
void processMessage(const T& message)
{

}

Device::Device(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const caf::actor& group, const mccmsg::ProtocolId & id, const std::string& name)
    :   caf::event_based_actor(cfg)
    , _name(name)
    , _id(id)
    , _device(id.device())
    , _helper(id, core, this)
    , _core(core)
    , _broker(broker)
    , _group(group)
    , _stats(id.device())
    , _targetSystem((uint8_t)id.id())
    , _targetComponent(0)
    , _targetChannel(0)
    , _firstPacket(true)
    , _lastSequence(0)
{
    send(_group, mccnet::group_dev::value, id, caf::actor_cast<caf::actor>(this));
    _isConnected = false;
    _isActive = false;
    _routeController = createTrait<TraitRoutes>(".route");
    MavlinkSettings settings(id);
    _traitReg = createTrait<TraitRegistrator>(".reg", settings);
    _traitParams = createTrait<TraitParams>(".params", settings);
    _traitCalibration = createTrait<TraitSensorCalibration>(".cal", settings);
    _traitRadioCalibration = createTrait<TraitRadioCalibration>(".radcal", settings);
    _traitJoystick = createTrait<TraitJoystick>(".joystick", settings);

    m_mavlink_status[0].flags |= MAVLINK_STATUS_FLAG_OUT_MAVLINK1;
}

const char* Device::name() const
{
    return _name.c_str();
}

void Device::on_exit()
{
    destroy(_core);
    destroy(_group);
    destroy(_broker);
    destroy(_traitReg);
    destroy(_traitParams);
    destroy(_traitCalibration);
    destroy(_traitRadioCalibration);
    destroy(_routeController);
    destroy(_traitJoystick);
}

bmcl::OptionRc<const MavlinkMessageRc> packetToMavlinkMsg(mccmsg::PacketPtr&& pkt)
{
    mavlink_message_t bufferMessage = {};
    mavlink_status_t bufferStatus = {};
    mavlink_message_t outMessage = {};
    mavlink_status_t outStatus = {};

    auto reader = bmcl::MemReader(*pkt);
    if (reader.size() < 6)
        return bmcl::None;
    while (reader.sizeLeft() > 0)
    {
        if (mavlink_frame_char_buffer(&bufferMessage, &bufferStatus, reader.readUint8(), &outMessage, &outStatus) != MAVLINK_FRAMING_OK)
            continue;
        assert(reader.sizeLeft() == 0);
        return bmcl::makeRc<const MavlinkMessageRc>(outMessage);
    }
    return bmcl::None;
}

class ParamsVisitor : public mccmsg::CmdVisitor
{
public:
    ParamsVisitor(Device* self, mccnet::CmdPtr&& cmd)
        : mccmsg::CmdVisitor([&](const mccmsg::DevReq*) { _cmd->sendFailed(mccmsg::Error::CmdUnknown);})
        , _self(self)
        , _cmd(std::move(cmd))
    {
    }

    using mccmsg::CmdVisitor::visit;

    void visit(const mccmsg::CmdParamList* msg) override { _self->execute(std::move(_cmd), msg); }
    void visit(const mccmsg::CmdCalibrationStart* msg) override { _self->execute(std::move(_cmd), msg); }
    void visit(const mccmsg::CmdCalibrationCancel* msg) override { _self->execute(std::move(_cmd), msg); }
    void visit(const mccmsg::CmdRouteSet* cmd) override { _self->delegate(_self->_routeController, bmcl::wrapRc(cmd)); }
    void visit(const mccmsg::CmdParamRead* cmd) override { _self->delegate(_self->_traitParams, bmcl::wrapRc(cmd)); }
    void visit(const mccmsg::CmdParamWrite* cmd) override { _self->delegate(_self->_traitParams, bmcl::wrapRc(cmd)); }
private:
    Device* _self;
    mccnet::CmdPtr _cmd;
};

caf::behavior Device::make_behavior()
{
    using timer_state_atom = caf::atom_constant<caf::atom("timerstate")>;

    send(this, timer_state_atom::value);

    return
    {
        [this](rbt_cmd_atom)
        {
            BMCL_WARNING() << "Airframe is set. Rebooting...";
            sendCommand(MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN, 0, 1.0f);

            BMCL_WARNING() << "deactivating...";
            setActivated(false);
        }
      , [this](const mccmsg::device::Activate_RequestPtr msg) -> caf::result<mccmsg::device::Activate_ResponsePtr>
        {
            setActivated(msg->data()._state);
            return mccmsg::make<mccmsg::device::Activate_Response>(msg.get());
        }
      , [this](const mccmsg::device::UpdatedPtr&)
        {

        }
      , [this](const mccmsg::DevReqPtr& msg) -> caf::result<mccmsg::CmdRespAnyPtr>
        {
//            const auto& trait = msg->trait();
//             if (trait == "Navigation.Routes")
//             {
//
//             }
//             else if (trait == "Params")
//             {
//                 delegate(_traitParams, msg);
//                 return caf::delegated<mccmsg::CmdRespAnyPtr>();
//             }
//             else if (trait == "Joystick")
//             {
//                 delegate(_traitJoystick, msg);
//                 return caf::delegated<mccmsg::CmdRespAnyPtr>();
//             }

            ParamsVisitor visitor{ this, bmcl::makeRc<mccnet::Cmd>(make_response_promise(), msg) };
            msg->visit(&visitor);
            return caf::delegated<mccmsg::CmdRespAnyPtr>();
        }
      , [this](const mccmsg::CancelPtr& msg)
        {
            BMCL_DEBUG() << "please, cancel smth here!" << msg->requestId();
            assert(false);
        }
      , [this](mccnet::connected_atom)
        {
            if (_isConnected) return;
            _helper.log_text(bmcl::LogLevel::Info, "доступны каналы обмена");
            _isConnected = true;
            state_changed();
        }
      , [this](mccnet::disconnected_atom)
        {
            if (!_isConnected) return;
            _helper.log_text(bmcl::LogLevel::Warning, "недоступны каналы обмена");
            _isConnected = false;
            state_changed();
        }
      , [this](mccmsg::PacketPtr& pkt)
        {
            if (!_isActive)
                return;

            processMavlinkPacket(std::move(pkt));
            _stats._rcvd.add(pkt->size(), 1);
        }
      , [this](mccnet::req_atom, mccmsg::PacketPtr& pkt)
        {
            return delegate(_broker, mccnet::req_atom::value, _device, std::move(pkt));
        }
      , [this](send_msg_atom, const MavlinkMessagePtr& msg)
        {
            sendMavlinkMessageToChannel(msg);
        }
      , [this](waypoint_msg_atom, const MavlinkMessagePtr& msg, int targetMsg)
        {
            auto it = _pendingMessageRequests.find(targetMsg);
            if (it != _pendingMessageRequests.end())
            {
                BMCL_WARNING() << "it != _pendingRequests.end()" << msg->msgid;
                _pendingMessageRequests.erase(it);
            }

            caf::response_promise rp = make_response_promise();
            _pendingMessageRequests.emplace(targetMsg, std::move(rp));

            sendMavlinkMessageToChannel(msg);
        }
      , [this](waypoint_msg_no_rep_atom, const MavlinkMessagePtr& msg)
        {
            sendMavlinkMessageToChannel(msg);
        }
      , [this](timer_state_atom)
        {
            sendStateTm();
            delayed_send(this, std::chrono::seconds(1), timer_state_atom::value);
        }
      , [this](frmupdate_atom, const FirmwarePtr& frm)
        {
            _firmware = frm;
            _helper.log(bmcl::LogLevel::Info, fmt::format("Аппарат {} зарегистрирован. Становится активным.", _id.id()));
            send(_broker, activated_atom::value, _helper.device(), true);
            for (auto& i : _traits)
                send(i, frmupdate_atom::value, frm);
        }
       , [this](params_dump_atom, const std::vector<ParamValue>& params)
        {
            send(_traitParams, params_dump_atom::value, params);
        }
      , [this](set_param_atom, const std::string& id, const mccmsg::NetVariant& value)
        {
            send(_traitCalibration, set_param_atom::value, id, value);
        }
      , [this](write_param_atom, const MavlinkMessagePtr& msg, int paramIndex)
        {
            auto it = _pendingParamsRequests.find(paramIndex);
            if (it != _pendingParamsRequests.end())
            {
                BMCL_WARNING() << "it != _pendingRequests.end()" << msg->msgid;
                _pendingParamsRequests.erase(it);
            }

            caf::response_promise rp = make_response_promise();
            _pendingParamsRequests.emplace(paramIndex, std::move(rp));

            sendMavlinkMessageToChannel(msg);
        }
      , [this](mccnet::activated_list_atom, const std::vector<std::size_t>& devs)
        {
            _activeDevices = devs;
            std::sort(_activeDevices.begin(), _activeDevices.end());
        }
      , [this](mccnet::group_cmd_new, const mccmsg::Group& group, const std::vector<std::size_t>& devs)
        {
            return mccmsg::make_error(mccmsg::Error::NotImplemented);
        }
      , [this](mccnet::group_cmd_del, const mccmsg::Group& group)
        {
            return mccmsg::make_error(mccmsg::Error::NotImplemented);
        }
      , [this](mccnet::group_cmd_att, const mccmsg::Group& group, std::size_t dev)
        {
            return mccmsg::make_error(mccmsg::Error::NotImplemented);
        }
      , [this](mccnet::group_cmd_det, const mccmsg::Group& group, std::size_t dev)
        {
            return mccmsg::make_error(mccmsg::Error::NotImplemented);
        }
      , [this](mccnet::group_cmd, const mccmsg::Group& , const mccmsg::DevReqPtr&)
        {
            return mccmsg::make_error(mccmsg::Error::NotImplemented);
        }
    };
}

void Device::state_changed()
{
    bool state = _isActive && _isConnected;
    if (state)
        _firstPacket = true;

    for (auto& i : _traits)
    {
        if (state)
            send(i, activated_atom::value);
        else
            send(i, deactivated_atom::value);
    }

    sendStateTm();
}

void Device::setActivated(bool state)
{
    if (_isActive != state)
    {
        _isActive = state;
        _helper.log_text(bmcl::LogLevel::Info, _isActive ? "активировано" : "деактивировано");
        state_changed();
    }
    if (!_isActive)
    {
        _firmware.reset();
        send(_broker, activated_atom::value, _helper.device(), false);
    }
}

void Device::execute(mccnet::CmdPtr&& rp, mccmsg::CmdParamListPtr&& cmd)
{
    const std::string & trait = cmd->trait();
    const std::string & command = cmd->command();

    if (_firmware.isNull() || !_firmware->isPx4())
    {
        rp->sendFailed("Прошивка не выгружена или не PX4!");
        return;
    }
    if (trait == "Device")
    {
        if (command == "setMode")
        {
            auto newMode = DeviceState::fromString(cmd->params().at(0).asQString());
            if (newMode.isNone())
            {
                rp->sendFailed("Неизвестный режим");
                return;
            }

            BMCL_DEBUG() << "Switching to : " << newMode->toString() << newMode->baseMode() << newMode->customMode();
            setMode(newMode->baseMode(), newMode->customMode());
        }
        else if (command == "setArmed")
        {
            setArmed(cmd->params().at(0).toUint() == 1);
        }
        else if (command == "emergency")
        {
            setEmergency();
        }
        else if (command == "takeOff")
        {
            takeOff();
        }
        else if (command == "land")
        {
            land();
        }
        rp->sendDone();
        return;
    }
    if (trait == "System")
    {
        if (command == "setAirframe")
        {
            uint32_t airframe = cmd->params()[0].toUint();
            std::vector<std::pair<std::string, mccmsg::NetVariant>> vars;
            vars.emplace_back("SYS_AUTOSTART", airframe);
            vars.emplace_back("SYS_AUTOCONFIG", 1);

            auto req = bmcl::makeRc<const mccmsg::CmdParamWrite>(_id.device(), "", vars);

            request(_traitParams, caf::infinite, std::move(req)).then
            (
                [this](const mccmsg::CmdRespAnyPtr& p)
                {
                    delayed_send(this, std::chrono::milliseconds(1000), rbt_cmd_atom::value);
                }
              , [this, airframe](const caf::error& e)
                {
                    BMCL_CRITICAL() << "Failed to set airframe: " << airframe;
                }
            );
        }
    }
}

void Device::execute(mccnet::CmdPtr&& cmd, mccmsg::CmdCalibrationStartPtr&& msg)
{
    if(msg->component() == mccmsg::CalibrationSensor::Radio)
        send(_traitRadioCalibration, msg);
    else
        send(_traitCalibration, msg);

    cmd->sendDone();
}

void Device::execute(mccnet::CmdPtr&& cmd, mccmsg::CmdCalibrationCancelPtr&& msg)
{
    send(_traitCalibration, msg);
    send(_traitRadioCalibration, msg);

    cmd->sendDone();
}

void Device::sendCommand(uint16_t cmdId, uint8_t confirmation, float param1, float param2, float param3, float param4,
                         float param5, float param6, float param7)
{
    sendCommandWithSystemComponent(cmdId, _targetSystem, _targetComponent, confirmation, param1, param2, param3, param4,
                                   param5, param6, param7);
}

void Device::sendCommandWithSystemComponent(uint16_t cmdId, uint8_t targetSystem, uint8_t targetComponent,
                                            uint8_t confirmation, float param1, float param2, float param3,
                                            float param4, float param5, float param6, float param7)
{
    MavlinkMessageRc* msg = new MavlinkMessageRc;
    mavlink_msg_command_long_pack_chan(_targetSystem, _targetComponent, _targetChannel, msg, targetSystem, targetComponent, cmdId, confirmation,
                                       param1, param2, param3, param4, param5, param6, param7);
    sendMavlinkMessageToChannel(MavlinkMessagePtr(msg));
}

void Device::setMode(uint8_t base_mode, uint32_t custom_mode)
{
    uint8_t newBaseMode = _lastHeartbeat.base_mode & ~MAV_MODE_FLAG_DECODE_POSITION_CUSTOM_MODE;
    newBaseMode |= base_mode;

    MavlinkMessageRc* msg = new MavlinkMessageRc;
    mavlink_msg_set_mode_pack_chan(_targetSystem,
                                   _targetComponent,
                                   _targetChannel,
                                   msg,
                                   (uint8_t)_id.id(),
                                   newBaseMode,
                                   custom_mode);
    sendMavlinkMessageToChannel(MavlinkMessagePtr(msg));
}

void Device::setArmed(bool armed)
{
    sendCommand(MAV_CMD_COMPONENT_ARM_DISARM, 0, armed ? 1.0f : 0.0f);
}

void Device::setEmergency()
{
    sendCommand(MAV_CMD_COMPONENT_ARM_DISARM, 0, 0.0f, 21196.0f, 0.0, 0.0, 0.0, 0.0, 0.0);
}

void Device::takeOff()
{
    sendCommand(MAV_CMD_NAV_TAKEOFF, 0, -1.0f, 0.0, 0.0);
}

void Device::land()
{
    auto newMode = DeviceState::fromString("Land");
    if (newMode.isNone())
    {
        Q_ASSERT(false);
        return;
    }
    setMode(newMode->baseMode(), newMode->customMode());
}

void Device::processMavlinkMessageHeartbeat(const MavlinkMessagePtr& mavlinkMessage)
{
    mavlink_heartbeat_t heartbeat;
    mavlink_msg_heartbeat_decode(mavlinkMessage.get(), &heartbeat);
    _lastHeartbeat = heartbeat;
    _helper.sendTrait(new TmUpdateMode(_id.device(), heartbeat.base_mode, heartbeat.custom_mode, heartbeat.system_status));
    send(_traitReg,  heartbeat);
}

void Device::processMavlinkMessageStatusText(const MavlinkMessagePtr& mavlinkMessage)
{
    QByteArray b;
    b.resize(MAVLINK_MSG_STATUSTEXT_FIELD_TEXT_LEN + 1);
    mavlink_msg_statustext_get_text(mavlinkMessage.get(), b.data());

    // Ensure NUL-termination
    b[b.length() - 1] = '\0';
    QString text = QString(b);
    bmcl::LogLevel logLevel = bmcl::LogLevel::Debug;
    int severity = mavlink_msg_statustext_get_severity(mavlinkMessage.get());
    if(severity == MAV_SEVERITY_EMERGENCY)
        logLevel = bmcl::LogLevel::Critical;
    else if(severity <= MAV_SEVERITY_ERROR)
        logLevel = bmcl::LogLevel::Critical;
    else if(severity == MAV_SEVERITY_WARNING)
        logLevel = bmcl::LogLevel::Warning;
    else if(severity <= MAV_SEVERITY_INFO)
        logLevel = bmcl::LogLevel::Info;
    else
        logLevel = bmcl::LogLevel::Debug;

    _helper.log_text(logLevel, text.toStdString());
//     _helper.sendTm(mccmsg::TmParams{ { "Device", "statusText", text } });

    send(_traitCalibration, text.toStdString());
}

void Device::processMavlinkMessageAutopilotVersion(const MavlinkMessagePtr& mavlinkMessage)
{
    BMCL_DEBUG() << "processMavlinkMessageAutopilotVersion";
    mavlink_autopilot_version_t version;
    mavlink_msg_autopilot_version_decode(mavlinkMessage.get(), &version);

//     uint64_t capabilities           = params.at("capabilities").toUint64();
//     uint64_t uid                    = params.at("uid").toUint64();
//     uint32_t flight_sw_version      = params.at("flightSwVersion").toUint32();
//     uint32_t middleware_sw_version  = params.at("middlewareSwVersion").toUint32();
//     uint32_t os_sw_version          = params.at("osSwVersion").toUint32();
//     uint32_t board_version          = params.at("boardVersion").toUint32();
//     uint16_t vendor_id              = params.at("vendorId").toUint16();
//     uint16_t product_id             = params.at("productId").toUint16();

    //                     if (flight_sw_version != 0) {
    //                         int majorVersion, minorVersion, patchVersion;
    //                         FIRMWARE_VERSION_TYPE versionType;
    //
    //                         majorVersion = (autopilotVersion.flight_sw_version >> (8 * 3)) & 0xFF;
    //                         minorVersion = (autopilotVersion.flight_sw_version >> (8 * 2)) & 0xFF;
    //                         patchVersion = (autopilotVersion.flight_sw_version >> (8 * 1)) & 0xFF;
    //                         versionType = (FIRMWARE_VERSION_TYPE)((autopilotVersion.flight_sw_version >> (8 * 0)) & 0xFF);
    //                         setFirmwareVersion(majorVersion, minorVersion, patchVersion, versionType);
    //                     }

}

void Device::processMavlinkMessageParamValue(const MavlinkMessagePtr& message)
{
    send(_traitReg, message);

    mavlink_param_value_t rawValue;
    mavlink_msg_param_value_decode(message.get(), &rawValue);
    send(_traitParams, rawValue);

    auto it = _pendingParamsRequests.find(rawValue.param_index);
    if (it != _pendingParamsRequests.end())
    {
        it->second.deliver(message);
        _pendingParamsRequests.erase(it);
    }
}

void Device::processMavlinkMessageSysStatus(const MavlinkMessagePtr& mavlinkMessage)
{
    uint8_t batteryLevel = mavlink_msg_sys_status_get_battery_remaining(mavlinkMessage.get());
     if (batteryLevel > 100)
         batteryLevel = 0;
//      mccmsg::TmParams params1 = {
//          { "Device", "batteryLevel", batteryLevel },
//      };
/*     _deviceState.battery = batteryLevel;*/
//      _helper.sendTm(std::move(params1));
}

void Device::processMavlinkMessageAttitude(const MavlinkMessagePtr& mavlinkMessage)
{
//     mavlink_attitude_t attitude;
//     mavlink_msg_attitude_decode(mavlinkMessage.get(), &attitude);
// 
//     _motion.orientation = mccgeo::Attitude(bmcl::radiansToDegrees(attitude.yaw),
//                                               bmcl::radiansToDegrees(attitude.pitch),
//                                               bmcl::radiansToDegrees(attitude.roll));
//     _helper.sendTrait(new mccmsg::TmMotion(_id.device(), _motion));
}

void Device::processMavlinkMessageGlobalPositionInt(const MavlinkMessagePtr& mavlinkMessage)
{
//     mavlink_global_position_int_t position;
//     mavlink_msg_global_position_int_decode(mavlinkMessage.get(), &position);

//     mavlink_gps_raw_int_t position;
//     mavlink_msg_gps_raw_int_decode(&mavlinkMessage, &position);

//     double latitudeDeg = position.lat / 10000000.0;
//     double longitudeDeg = position.lon / 10000000.0;
//     double altitudeM = position.alt / 1000.0;
//     double relAlt = position.relative_alt / 1000.0;
// 
//     // TODO: это три состовляющих скорости в метрах в секунду, их надо использовать
//     double vxMps = position.vx / 100.0;
//     double vyMps = position.vy / 100.0;
//     double vzMps = position.vz / 100.0;
//     double speed = std::hypot(std::hypot(vxMps, vyMps), vzMps);
// 
// //    _motion.position = mccgeo::Position(latitudeDeg, longitudeDeg, altitudeM);
//     _motion.velocity = mccgeo::Position(vxMps, vyMps, vzMps);
//     _motion.speed = speed;
//     _motion.relativeAltitude = relAlt;
//     _helper.sendTrait(new mccmsg::TmMotion(_id.device(), _motion));
}

void Device::processMavlinkMessageCommandAck(const MavlinkMessagePtr& mavlinkMessage)
{
    uint16_t cmd = mavlink_msg_command_ack_get_command(mavlinkMessage.get());
    MAV_RESULT result = static_cast<MAV_RESULT>(mavlink_msg_command_ack_get_result(mavlinkMessage.get()));
    BMCL_DEBUG() << "Command" << cmd << resultToString(result);
}

void Device::processMavlinkMessageGpsRawInt(const MavlinkMessagePtr& mavlinkMessage)
{
//     using mccmsg::GpsFixType;
// 
//     mavlink_gps_raw_int_t gps;
//     mavlink_msg_gps_raw_int_decode(mavlinkMessage.get(), &gps);
// 
//     double latitudeDeg = gps.lat / 10000000.0;
//     double longitudeDeg = gps.lon / 10000000.0;
//     double altitudeM = gps.alt / 1000.0;
// 
//     _motion.position = mccgeo::Position(latitudeDeg, longitudeDeg, altitudeM);
// 
//     bmcl::Option<GpsFixType> fix_type;
//     switch (gps.fix_type)
//     {
//     case GPS_FIX_TYPE_NO_GPS: fix_type = GpsFixType::NoGps; break;
//     case GPS_FIX_TYPE_NO_FIX: fix_type = GpsFixType::NoFix; break;
//     case GPS_FIX_TYPE_2D_FIX: fix_type = GpsFixType::Fix2D; break;
//     case GPS_FIX_TYPE_3D_FIX: fix_type = GpsFixType::Fix3D; break;
//     case GPS_FIX_TYPE_DGPS: fix_type = GpsFixType::DGps; break;
//     case GPS_FIX_TYPE_RTK_FLOAT: fix_type = GpsFixType::L1Float; break; /* RTK float, 3D position | */
//     case GPS_FIX_TYPE_RTK_FIXED: fix_type = GpsFixType::L1Int; break; /* RTK Fixed, 3D position | */
//     case GPS_FIX_TYPE_STATIC: fix_type = GpsFixType::Static; break;
//     default:
//         BMCL_WARNING() << "Unknown gps fix type: " << gps.fix_type;
//     }
}

void Device::processMavlinkGpsStatus(const MavlinkMessagePtr& mavlinkMessage)
{
    using mccmsg::GpsSat;
    using mccmsg::GpsSats;

    mavlink_gps_status_t gps;
    mavlink_msg_gps_status_decode(mavlinkMessage.get(), &gps);
    uint count = std::min<uint8_t>(gps.satellites_visible, sizeof(gps.satellite_prn) / sizeof(gps.satellite_prn[0]));

    GpsSats sats;
    sats.reserve(count);
    for (uint i = 0; i < sizeof(gps.satellite_prn) / sizeof(gps.satellite_prn[0]); ++i)
    {
        sats.emplace_back(GpsSat(gps.satellite_prn[i], gps.satellite_elevation[i], gps.satellite_azimuth[i], gps.satellite_snr[i], gps.satellite_used[i]));
    }
}

void Device::processMavlinkMessageRcChannels(const MavlinkMessagePtr& mavlinkMessage)
{
    mavlink_rc_channels_t channels;
    mavlink_msg_rc_channels_decode(mavlinkMessage.get(), &channels);
    send(_traitRadioCalibration, channels);
}

void Device::sendStateTm()
{
    _stats._isActive = _isActive;
//    _stats._regState = 100;
    _helper.sendStats(_stats);
}

void Device::processMavlinkPacket(mccmsg::PacketPtr&& pkt)
{
    auto r = packetToMavlinkMsg(std::move(pkt));
    if (r.isNone())
        return;

    MavlinkMessagePtr message = r.take();
    _helper.sendTrait(new TmUpdateMsg(_id.device(), message));

    //sendTm(message);
    //_helper.sendTrait(new TmUpdateMalvinkMessage(_helper.device(), message));

//    checkSequenceCounter(message);

    switch (message->msgid)
    {
    case MAVLINK_MSG_ID_HEARTBEAT:              processMavlinkMessageHeartbeat(message);         break;
    case MAVLINK_MSG_ID_STATUSTEXT:             processMavlinkMessageStatusText(message);        break;
    case MAVLINK_MSG_ID_AUTOPILOT_VERSION:      processMavlinkMessageAutopilotVersion(message);  break;
    case MAVLINK_MSG_ID_PARAM_VALUE:            processMavlinkMessageParamValue(message);        break;
    case MAVLINK_MSG_ID_SYS_STATUS:             processMavlinkMessageSysStatus(message);         break;
    case MAVLINK_MSG_ID_ATTITUDE:               processMavlinkMessageAttitude(message);          break;
    case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:    processMavlinkMessageGlobalPositionInt(message); break;
    case MAVLINK_MSG_ID_COMMAND_ACK:            processMavlinkMessageCommandAck(message);        break;
    case MAVLINK_MSG_ID_GPS_RAW_INT:            processMavlinkMessageGpsRawInt(message);         break;
    case MAVLINK_MSG_ID_RC_CHANNELS:            processMavlinkMessageRcChannels(message);        break;
    case MAVLINK_MSG_ID_GPS_STATUS:             processMavlinkGpsStatus(message);                break;
    case MAVLINK_MSG_ID_MISSION_ITEM_REACHED:   send(_routeController, message);                 break;
    case MAVLINK_MSG_ID_MISSION_CURRENT:        send(_routeController, message);                 break;
    }

    auto it = _pendingMessageRequests.find(message->msgid);
    if (it != _pendingMessageRequests.end())
    {
        it->second.deliver(message);
        BMCL_DEBUG() << message->msgid;

        _pendingMessageRequests.erase(it);
    }
}

void Device::sendTm(const MavlinkMessagePtr &message)
{
    auto messageinfo = mavlink_get_message_info(message.get());
    std::string trait = messageinfo->name;
//     uint8_t* payload = (uint8_t*)(message->payload64);
//     mccmsg::TmParams tmParams;
//     for (std::size_t i = 0; i < messageinfo->num_fields; ++i)
//     {
//         mavlink_field_info_t field = messageinfo->fields[i];
//         std::string name = field.name;
//         mccmsg::NetVariant value;
//         if (field.array_length > 0)
//             continue;
//
//         switch (field.type)
//         {
//             case MAVLINK_TYPE_UINT8_T:
//                 value = *((uint8_t*)(payload + field.wire_offset));
//                 break;
//             case MAVLINK_TYPE_INT8_T:
//                 value = *((int8_t*)(payload + field.wire_offset));
//                 break;
//             case MAVLINK_TYPE_UINT16_T:
//                 value = *((uint16_t*)(payload + field.wire_offset));
//                 break;
//             case MAVLINK_TYPE_INT16_T:
//                 value = *((int16_t*)(payload + field.wire_offset));
//                 break;
//             case MAVLINK_TYPE_UINT32_T:
//                 value = *((uint32_t*)(payload + field.wire_offset));
//                 break;
//             case MAVLINK_TYPE_INT32_T:
//                 value = *((int32_t*)(payload + field.wire_offset));
//                 break;
//             case MAVLINK_TYPE_FLOAT:
//                 value = *((float*)(payload + field.wire_offset));
//                 break;
//             case MAVLINK_TYPE_DOUBLE:
//                 value = *((double*)(payload + field.wire_offset));
//                 break;
//             case MAVLINK_TYPE_UINT64_T:
//                 value = *((uint64_t*)(payload + field.wire_offset));
//                 break;
//             case MAVLINK_TYPE_INT64_T:
//                 value = *((int64_t*)(payload + field.wire_offset));
//                 break;
// //            default:
//                 //BMCL_WARNING() << "WARNING: UNKNOWN MAVLINK TYPE";
//         }
//
//         tmParams.emplace_back(mccmsg::TmParam(trait, name, value));
//     }

//     _helper.sendTm(std::move(tmParams));
}

void Device::checkSequenceCounter(const MavlinkMessagePtr& message)
{
    if (_firstPacket)
    {
        _firstPacket = false;
        _lastSequence = message->seq;
        return;
    }
    if (message->seq - _lastSequence > 1)
    {
//        BMCL_WARNING() << "Потерян пакет. Последний: " << _lastSequence << "Текущий: " << message.seq;
        _helper.log(bmcl::LogLevel::Debug, fmt::format("Потерян пакет. Последний: {}, текущий: {}", _lastSequence, message->seq));
    }

    _lastSequence = message->seq;
}
}
