#include "mcc/msg/Packet.h"
#include "../traits/TraitRegistrator.h"
#include "../device/MavlinkUtils.h"
#include "../traits/Trait.h"
#include "../Firmware.h"
#include "mcc/msg/ptr/Device.h"
#include "mcc/msg/ptr/Firmware.h"
#include "mcc/res/Resource.h"

#include <fmt/format.h>
#include <bmcl/Logging.h>
#include <bmcl/MakeRc.h>

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmav::MavlinkMessagePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mavlink_heartbeat_t);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(std::vector<mccmav::ParamValue>);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmav::FirmwarePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::DbReqPtr);

namespace mccmav {

const char* toString(MAV_AUTOPILOT ap)
{
    switch (ap)
    {
    case MAV_AUTOPILOT_GENERIC:                                      return "generic";
    case MAV_AUTOPILOT_RESERVED:                                     return "reserved";
    case MAV_AUTOPILOT_SLUGS:                                        return "slugs";
    case MAV_AUTOPILOT_ARDUPILOTMEGA:                                return "ardupilotmega";
    case MAV_AUTOPILOT_OPENPILOT:                                    return "openpilot";
    case MAV_AUTOPILOT_GENERIC_WAYPOINTS_ONLY:                       return "simplewaypoints";
    case MAV_AUTOPILOT_GENERIC_WAYPOINTS_AND_SIMPLE_NAVIGATION_ONLY: return "simplenavigationonly";
    case MAV_AUTOPILOT_GENERIC_MISSION_FULL:                         return "missionfull";
    case MAV_AUTOPILOT_INVALID:                                      return "invalid";
    case MAV_AUTOPILOT_PPZ:                                          return "ppz";
    case MAV_AUTOPILOT_UDB:                                          return "uav";
    case MAV_AUTOPILOT_FP:                                           return "flexipilot";
    case MAV_AUTOPILOT_PX4:                                          return "px4";
    case MAV_AUTOPILOT_SMACCMPILOT:                                  return "smaccmpilot";
    case MAV_AUTOPILOT_AUTOQUAD:                                     return "autoquad";
    case MAV_AUTOPILOT_ARMAZILA:                                     return "armazila";
    case MAV_AUTOPILOT_AEROB:                                        return "aerob";
    case MAV_AUTOPILOT_ASLUAV:                                       return "asluav";
    default:
        return "unknown";
    }
}

const char* toString(MAV_TYPE t)
{
    switch (t)
    {
    case MAV_TYPE_GENERIC: return "plane";
    case MAV_TYPE_FIXED_WING: return "flying_wing";
    case MAV_TYPE_QUADROTOR: return "quadcopter";
    case MAV_TYPE_ROCKET: return "missile";
    case MAV_TYPE_GROUND_ROVER: return "car";
    case MAV_TYPE_SUBMARINE: return "submarine";
    case MAV_TYPE_FLAPPING_WING: return "paragliding";
    default: return "unknown";
    }
}

bmcl::SharedBytes toPixmap(MAV_TYPE t)
{
    switch (t)
    {
    case MAV_TYPE_GENERIC: return bmcl::SharedBytes::create(mccres::loadResource(mccres::ResourceKind::DevicePlaneIcon));
    case MAV_TYPE_FIXED_WING: return bmcl::SharedBytes::create(mccres::loadResource(mccres::ResourceKind::DeviceFlyingWingIcon));
    case MAV_TYPE_QUADROTOR: return bmcl::SharedBytes::create(mccres::loadResource(mccres::ResourceKind::DeviceQuadcopterIcon));
    case MAV_TYPE_ROCKET: return bmcl::SharedBytes::create(mccres::loadResource(mccres::ResourceKind::DeviceMissileIcon));
    case MAV_TYPE_GROUND_ROVER: return bmcl::SharedBytes::create(mccres::loadResource(mccres::ResourceKind::DeviceCarIcon));
    case MAV_TYPE_SUBMARINE: return bmcl::SharedBytes::create(mccres::loadResource(mccres::ResourceKind::DeviceSubmarineIcon));
    case MAV_TYPE_FLAPPING_WING: return bmcl::SharedBytes::create(mccres::loadResource(mccres::ResourceKind::DeviceParaglidingIcon));
    default: return bmcl::SharedBytes::create(mccres::loadResource(mccres::ResourceKind::DeviceUnknownIcon));
    }
}

TraitRegistrator::TraitRegistrator(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const mccmsg::ProtocolId& id, const std::string& name, const MavlinkSettings& settings)
    : caf::event_based_actor(cfg), _helper(id, core, this), _id(id), _activated(false), _name(name), _core{ core }, _broker{ broker }, _settings(settings), _paramsCount(0)
{
}

void TraitRegistrator::on_exit()
{
    removeFirmware();
}

caf::behavior TraitRegistrator::make_behavior()
{
    using timer_heartbeat_atom = caf::atom_constant<caf::atom("heartbeat")>;

    request(_core, caf::infinite, mccmsg::makeReq(new mccmsg::device::Description_Request(_id.device()))).then
    (
        [this](const mccmsg::device::Description_ResponsePtr& rep)
        {
            if (!_mccFirmware.isNull()) return;

            auto name = rep->data()->firmware();
            if (name.isNone())
            {
                BMCL_DEBUG() << "Прошивка устройства не задана";
                return;
            }

            request(_core, caf::infinite, mccmsg::makeReq(new mccmsg::firmware::Description_Request(name.unwrap()))).then
            (
                [this](const mccmsg::firmware::Description_ResponsePtr& rep)
                {
                    if (!_mccFirmware.isNull())
                        return;
                    registered(rep->data());
                }
                , [this](const caf::error& e)
                {
                    BMCL_DEBUG() << "failed to get firmware description from db" << system().render(e).c_str();
                }
            );
        }
      , [this](const caf::error& e)
        {
            BMCL_DEBUG() << "failed to get device description from db" << system().render(e);
        }
    );

    send(this, timer_heartbeat_atom::value);

    return
    {
        [this](timer_heartbeat_atom)
        {
            if (_activated)
            {
                MavlinkMessageRc* msg = new MavlinkMessageRc;
                mavlink_msg_heartbeat_pack_chan(_settings.system,
                                                _settings.component,
                                                _settings.channel,
                                                msg,
                                                MAV_TYPE_GCS,            // MAV_TYPE
                                                MAV_AUTOPILOT_INVALID,   // MAV_AUTOPILOT
                                                MAV_MODE_MANUAL_ARMED,   // MAV_MODE
                                                0,                       // custom mode
                                                MAV_STATE_ACTIVE);       // MAV_STATE

                send(_broker, send_msg_atom::value, MavlinkMessagePtr(msg));

                if (_state == State::WaitingParams && _downloadTimer.passed().count() > 3000)
                {
                    if (_downloadedParams.empty())
                        requestParams();
                    else
                        requestMissedParams();
                }
            }
            delayed_send(this, std::chrono::seconds(1), timer_heartbeat_atom::value);
        }
      , [this](const mavlink_heartbeat_t& msg)
        {
            if (!_activated)
                return;

            if (_state == State::WaitingHeartbeat)
            {
                _autopilotKind = (MAV_AUTOPILOT)msg.autopilot;
                _autopilotType = (MAV_TYPE)msg.type;

                sendProgress(1);

                requestAutopilotVersion();
            }
        }
      , [this](const MavlinkMessagePtr& message)
        {
            if (!_activated)
                return;

            handleParam(message);
        }
      , [this](activated_atom)
        {
            if (!_activated)
            {
                removeFirmware();
                _activated = true;
                _state = State::WaitingHeartbeat;
            }
        }
      , [this](deactivated_atom)
        {
            if (_activated)
            {
                _activated = false;
                _state = State::WaitingHeartbeat;
                removeFirmware();
            }
        }
      , [this](const mccmsg::DevReqPtr& msg)
        {
            return mccmsg::make_error(mccmsg::Error::CmdUnknown);
        }
      , [this](frmupdate_atom, const FirmwarePtr& frm)
        {

        }
    };
}

const char* TraitRegistrator::name() const
{
    return _name.c_str();
}

void TraitRegistrator::update_device(const mccmsg::FirmwareDescription& tmp)
{
    mccmsg::DeviceDescription d = new mccmsg::DeviceDescriptionObj(_id.device(), "", "", _id, bmcl::None, toPixmap(_autopilotType), tmp->name());
    mccmsg::device::Updater updater(std::move(d), {mccmsg::Field::Kind, mccmsg::Field::Firmware});
    request(_core, caf::infinite, mccmsg::makeReq(new mccmsg::device::Update_Request(std::move(updater)))).then
    (
        [this, tmp](const mccmsg::device::Update_ResponsePtr& resp) mutable { registered(tmp); }
      , [](const caf::error& err) {}
    );
}

void TraitRegistrator::requestAutopilotVersion()
{
    MavlinkMessageRc* msg = new MavlinkMessageRc;
    mavlink_command_long_t cmd;
    cmd.command = MAV_CMD_REQUEST_AUTOPILOT_CAPABILITIES;
    cmd.confirmation = 0;
    cmd.target_component = _settings.component;
    cmd.target_system = _settings.component;
    cmd.param1 = 0.0f;
    cmd.param2 = 0.0f;
    cmd.param3 = 0.0f;
    cmd.param4 = 0.0f;
    cmd.param5 = 0.0f;
    cmd.param6 = 0.0f;
    cmd.param7 = 0.0f;

    mavlink_msg_command_long_encode(_settings.system, _settings.component, msg, &cmd);

    _helper.log_text(bmcl::LogLevel::Info, "Запрос версии автопилота");
    _state = State::WaitingVersion;

    request(_broker, std::chrono::seconds(3), waypoint_msg_atom::value, MavlinkMessagePtr(msg), MAVLINK_MSG_ID_AUTOPILOT_VERSION).then(
        [this](const MavlinkMessagePtr& message)
        {
            mavlink_autopilot_version_t version;
            mavlink_msg_autopilot_version_decode(message.get(), &version);

            std::string name = toString(_autopilotKind);
            std::string type = toString(_autopilotType);
            _fwName = fmt::format("{}_{}_{:#08x}_{}", name, type, version.flight_sw_version, _id.id());
            _helper.log(bmcl::LogLevel::Info, fmt::format("Загрузили версию прошивки: {}", _fwName));
            _state = State::WaitingParams;

            load_frm(std::move(_fwName));
            requestParams();
        }
      , [this](const caf::error& err)
        {
            BMCL_DEBUG() << "Таймаут запроса версии автопилота: " << system().render(err);
            _state = State::WaitingHeartbeat;
        }
    );
}

void TraitRegistrator::requestParams()
{
    _paramsBuffer.clear();
    _downloadedParams.clear();
    sendProgress(0);

    _helper.log_text(bmcl::LogLevel::Info, "Начало выгрузки параметров автопилота...");
    _downloadTimer.start();

    MavlinkMessageRc* msg = new MavlinkMessageRc;
    mavlink_msg_param_request_list_pack_chan(_settings.system, _settings.component, _settings.channel, msg, _settings.system, MAV_COMP_ID_ALL);
    request(_broker, std::chrono::seconds(3), waypoint_msg_atom::value, MavlinkMessagePtr(msg), MAVLINK_MSG_ID_PARAM_VALUE).then(
        [this](const MavlinkMessagePtr& message)
        {
            handleParam(message);
        }
      , [this](const caf::error& err)
        {
            BMCL_DEBUG() << "requestParams: " << system().render(err);
        }
  );
}

void TraitRegistrator::handleParam(const MavlinkMessagePtr& message)
{
    mavlink_param_value_t rawValue;
    mavlink_msg_param_value_decode(message.get(), &rawValue);

    if (_paramsBuffer.empty())
    {
        _downloadedParams.clear();
        _paramsCount = rawValue.param_count;
        for (int i = 0; i < rawValue.param_count; ++i)
            _downloadedParams.insert(i);
    }

    if (_downloadedParams.find(rawValue.param_index) == _downloadedParams.end())
        return;

    _downloadedParams.erase(rawValue.param_index);

    mavlink_param_union_t paramVal;
    paramVal.param_float = rawValue.param_value;
    paramVal.type = rawValue.param_type;

    std::string sName;
    for (int i = 0; i < MAVLINK_MSG_PARAM_VALUE_FIELD_PARAM_ID_LEN; ++i)
    {
        if (rawValue.param_id[i] == 0)
            break;
        sName += rawValue.param_id[i];
    }

    ParamValue p;
    p.componentId = message->compid;
    p.name = sName;
    p.type = (MAV_PARAM_TYPE)paramVal.type;
    p.index = rawValue.param_index;
    p.value = toNetVariant(paramVal);
    _paramsBuffer.emplace_back(p);

    _downloadTimer.start();

    sendProgress((_paramsCount - (uint16_t)_downloadedParams.size()) * 100 / _paramsCount);

    if (_downloadedParams.empty())
    {
        sendProgress(100);
        applyFirmware();
    }
}

void TraitRegistrator::readParamByIndex(uint16_t index)
{
    MavlinkMessageRc* msg = new MavlinkMessageRc;
    mavlink_msg_param_request_read_pack_chan(_settings.system, _settings.component, _settings.channel, msg, _settings.system, _settings.component, "", index);

    send(_broker, send_msg_atom::value, MavlinkMessagePtr(msg));
}

void TraitRegistrator::registered(const mccmsg::FirmwareDescription& tmp)
{
    _mccFirmware = tmp;
    send(_broker, frmupdate_atom::value, bmcl::dynamic_pointer_cast<const Firmware>(_mccFirmware->frm()));
    send(_broker, params_dump_atom::value, _paramsBuffer);
}

void TraitRegistrator::load_frm(std::string&& name)
{
    if (!_mccFirmware.isNull() && _mccFirmware->frm()->id().value() == name)
        return;

    request(_core, caf::infinite, mccmsg::makeReq(new mccmsg::firmware::DescriptionS_Request(mccmsg::ProtocolValue(_id.protocol(), name)))).then
    (
        [this, name](mccmsg::firmware::DescriptionS_ResponsePtr& r) mutable
        {
            _helper.log(bmcl::LogLevel::Info, fmt::format("Загрузили описание прошивки из бд: {}", name));
            update_device(r->data());
        }
      , [this, name](const caf::error& err)
        {
            _helper.log_text(bmcl::LogLevel::Warning, "Не удалось загрузить описание прошивки из бд. Выгружаем с аппарата");
        }
    );
}

void TraitRegistrator::register_frm(mccmsg::FirmwareDescription&& tmp)
{
    if (!_mccFirmware.isNull() && _mccFirmware->frm()->id().value() == tmp->frm()->id().value())
        return;

    request(_core, caf::infinite, mccmsg::makeReq(new mccmsg::firmware::Register_Request(tmp))).then
    (
        [this, tmp](const mccmsg::firmware::Register_ResponsePtr& rep) mutable
        {
            update_device(rep->data());
        }
      , [this](const caf::error& err)
        {
        }
    );
}

void TraitRegistrator::applyFirmware()
{
    assert(_downloadedParams.empty());

    _state = State::Loaded;

    if (!_mccFirmware.isNull() && !_mccFirmware->frm().isNull() && _mccFirmware->frm()->id().value() == _fwName)
    {
        send(_broker, params_dump_atom::value, _paramsBuffer);
        return;
    }

    mccmsg::ProtocolValue pv(_helper.protocol(), _fwName);
    auto firmware = bmcl::makeRc<Firmware>(pv, _autopilotKind, _paramsBuffer, mccmsg::PropertyDescriptionPtrs(), mccmsg::PropertyDescriptionPtrs());
    auto firmwareDescription = bmcl::makeRc<const mccmsg::FirmwareDescriptionObj>(mccmsg::Firmware::createNil(), pv, firmware);
    register_frm(std::move(firmwareDescription));
}

void TraitRegistrator::removeFirmware()
{
    if (_mccFirmware.isNull())
        return;

    mccmsg::DeviceDescription d = new mccmsg::DeviceDescriptionObj(_id.device(), "", "", _id, bmcl::None, bmcl::SharedBytes(), bmcl::None);
    mccmsg::device::Updater u(std::move(d), {mccmsg::Field::Firmware});
    request(_core, caf::infinite, mccmsg::makeReq(new mccmsg::device::Update_Request(std::move(u))));

    std::string name = _mccFirmware->frm()->id().value();
    request(_core, caf::infinite, mccmsg::makeReq(new mccmsg::firmware::UnRegister_Request(_mccFirmware->name())));
    _helper.log(bmcl::LogLevel::Info, fmt::format("Удалена прошивка {}", name));
    _mccFirmware.reset();
}

void TraitRegistrator::requestMissedParams()
{
    assert(!_downloadedParams.empty());
    _helper.log(bmcl::LogLevel::Info, fmt::format("Дозапрос {} параметров", _downloadedParams.size()));
    for (auto p : _downloadedParams)
    {
        readParamByIndex(p);
    }
}

void TraitRegistrator::sendProgress(uint8_t progress)
{
    //send(_broker, regstate_atom::value, (uint8_t)progress);
}

}