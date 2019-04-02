#include <caf/make_message.hpp>
#include <caf/result.hpp>
#include <bmcl/MakeRc.h>
#include "mcc/msg/ptr/Device.h"
#include "mcc/msg/ptr/Channel.h"
#include "mcc/msg/FwdExt.h"
#include "mcc/net/db/obj/Device.h"
#include "mcc/net/db/obj/Channel.h"
#include "mcc/net/db/obj/DeviceUi.h"
#include "mcc/net/db/obj/Firmware.h"
#include "mcc/net/db/obj/Protocol.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::device::List_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::device::UnRegister_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::device::Connect_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::device::DescriptionList_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::device::Description_ResponsePtr);

namespace mccdb {
namespace dbobj {

bmcl::Result<bool, std::string> Device::updateFirmware(ObjectId device_id, const mccmsg::DeviceDescription& old, const bmcl::Option<mccmsg::Firmware>& firmware)
{
    if (old->firmware() == firmware)
        return false;

    _updateFirmware.reset();
    print(binds(&_updateFirmware, ":device_id", device_id));

    if (firmware.isSome())
    {
        auto r = _db->firmware().getId(firmware.unwrap());
        if (r.isNone())
            return std::string(mccmsg::to_string(mccmsg::Error::FirmwareUnknown));
        print(binds(&_updateFirmware, ":firmware_id", r.unwrap()));
    }
    else
        print(binds(&_updateFirmware, ":firmware_id", nullptr));

    SqlErrorX e = print(exec(&_updateFirmware));
    if (e.isSome()) return e.take();
    return true;
}

bmcl::Result<bool, std::string> Device::updateUi(ObjectId device_id, const mccmsg::DeviceDescription& old, const bmcl::Option<mccmsg::DeviceUi>& ui)
{
    if (old->ui() == ui)
        return false;

    _updateUi.reset();
    print(binds(&_updateUi, ":device_id", device_id));

    if (ui.isSome())
    {
        auto id = _db->deviceUi().getId(ui.unwrap());
        if (id.isNone())
            return std::string(mccmsg::to_string(mccmsg::Error::DeviceUiUnknown));
        print(binds(&_updateUi, ":device_ui_id", id));
    }

    SqlErrorX e = print(exec(&_updateUi));
    if (e.isSome()) return e.take();
    return true;
}

bmcl::Result<bool, std::string> Device::updatePixmap(ObjectId device_id, const mccmsg::DeviceDescription& old, const bmcl::SharedBytes& pixmap)
{
    if ( old->pixmap().view() == pixmap.view())
        return false;

    _updatePixmap.reset();
    print(binds(&_updatePixmap, ":device_id", device_id));

    if (pixmap.isNull())
        print(binds(&_updatePixmap, ":device_pixmap", nullptr));
    else
        print(binds(&_updatePixmap, ":device_pixmap", pixmap.view(), sqlite3pp::nocopy));

    SqlErrorX e = print(exec(&_updatePixmap));
    if (e.isSome()) return e.take();
    return true;
}

bmcl::Result<bool, std::string> Device::updateSettings(ObjectId device_id, const mccmsg::DeviceDescription& old, const std::string& settings)
{
    if (old->settings() == settings)
        return false;

    _updateSettings.reset();
    print(binds(&_updateSettings, ":device_id", device_id));
    print(binds(&_updateSettings, ":settings", settings, sqlite3pp::nocopy));
    SqlErrorX e = print(exec(&_updateSettings));
    if (e.isSome()) return e.take();
    return true;
}

bmcl::Result<bool, std::string> Device::updateInfo(ObjectId device_id, const mccmsg::DeviceDescription& old, const std::string& info)
{
    if (old->info() == info)
        return false;

    _updateInfo.reset();
    print(binds(&_updateInfo, ":device_id", device_id));
    print(binds(&_updateInfo, ":info", info, sqlite3pp::nocopy));
    SqlErrorX e = print(exec(&_updateInfo));
    if (e.isSome()) return e.take();
    return true;
}

bmcl::Result<bool, std::string> Device::updateReg(ObjectId device_id, const mccmsg::DeviceDescription& old, const bool& reg)
{
    if (old->registerFirst() == reg)
        return false;

    _updateReg.reset();
    print(binds(&_updateReg, ":device_id", device_id));
    print(binds(&_updateReg, ":reg_first", reg));
    SqlErrorX e = print(exec(&_updateReg));
    if (e.isSome()) return e.take();
    return true;
}

bmcl::Result<bool, std::string> Device::updateShow(ObjectId device_id, const mccmsg::DeviceDescription& old, const bool& show_on_map)
{
    if (old->showOnMap() == show_on_map)
        return false;

    _updateShow.reset();
    print(binds(&_updateShow, ":device_id", device_id));
    print(binds(&_updateShow, ":show_on_map", show_on_map));
    SqlErrorX e = print(exec(&_updateShow));
    if (e.isSome()) return e.take();
    return true;
}

bmcl::Result<bool, std::string> Device::updateLog(ObjectId device_id, const mccmsg::DeviceDescription& old, const bool& log)
{
    if (old->log() == log)
        return false;

    _updateLog.reset();
    print(binds(&_updateLog, ":device_id", device_id));
    print(binds(&_updateLog, ":log", log));
    SqlErrorX e = print(exec(&_updateLog));
    if (e.isSome()) return e.take();
    return true;
}

caf::result<mccmsg::device::Update_ResponsePtr> Device::execute(const mccmsg::device::Update_Request& request)
{
    const auto& d = request.data().dscr();
    ObjectId device_id;
    auto r = getOne(d->name(), &device_id);
    if (r.isErr())
        return r.takeErr();
    mccmsg::DeviceDescription old = r.take();

    sqlite3pp::transaction transaction(_db->db(), false);

    bool changed = false;
    for (auto i : request.data().fields())
    {
        bmcl::Result<bool, std::string> res = false;
        switch (i)
        {
        case mccmsg::Field::Firmware: res = updateFirmware(device_id, old, d->firmware()); break;
        case mccmsg::Field::Info: res = updateInfo(device_id, old, d->info()); break;
        case mccmsg::Field::Ui: res = updateUi(device_id, old, d->ui()); break;
        case mccmsg::Field::Kind: res = updatePixmap(device_id, old, d->pixmap()); break;
        case mccmsg::Field::RegFirst: res = updateReg(device_id, old, d->registerFirst()); break;
        case mccmsg::Field::ShowOnMap: res = updateShow(device_id, old, d->showOnMap()); break;
        case mccmsg::Field::Settings: res = updateSettings(device_id, old, d->settings()); break;
        case mccmsg::Field::Log: res = updateLog(device_id, old, d->log()); break;
        default:
            return mccmsg::make_error(mccmsg::Error::CantUpdate);
            break;
        }

        if (res.isErr())
            return mccmsg::make_error(mccmsg::Error::CantUpdate, res.takeErr());
        if (res.unwrap())
            changed = true;
    }

    if (changed)
    {
        transaction.commit();
        auto v = updated(d->name());
        if (v.isNone())
            return mccmsg::make_error(mccmsg::Error::CantUpdate);
    }

    return mccmsg::make<mccmsg::device::Update_Response>(&request/*, v.take()*/);
}

Device::Device(DbObjInternal* db)
    : QueryObject(db, "device", "settings, protocol_id, protocol_value, reg_first, show_on_map, device_pixmap, log, device_ui_id, firmware_id"
                , std::bind(&Device::get, this, std::placeholders::_1)
                , std::bind(&Device::insert, this, std::placeholders::_1, std::placeholders::_2))
    , _queryChannels(db->db())
    , _queryDisconnect(db->db())
    , _queryConnect(db->db())
    , _updateUi(db->db())
    , _updatePixmap(db->db())
    , _updateSettings(db->db())
    , _updateInfo(db->db())
    , _updateFirmware(db->db())
    , _updateReg(db->db())
    , _updateShow(db->db())
    , _updateLog(db->db())
    , _deleteFromChannel(db->db())
    , _deleteFromDevice(db->db())
{
    sql_prepare(_queryChannels, "select channel_id, channel.name as channel_name from device_channel, channel where device_channel.device_id = :device_id and device_channel.channel_id=channel.id");
    sql_prepare(_queryDisconnect, "delete from device_channel where device_id = :device_id and channel_id = :channel_id;");
    sql_prepare(_queryConnect, "insert or ignore into device_channel (device_id, channel_id) values (:device_id, :channel_id);");
    sql_prepare(_updateFirmware, "update device set firmware_id = :firmware_id where id = :device_id;");
    sql_prepare(_updateUi, "update device set device_ui_id = :device_ui_id where id = :device_id;");
    sql_prepare(_updatePixmap, "update device set device_pixmap = :device_pixmap where id = :device_id;");
    sql_prepare(_updateSettings, "update device set settings=:settings where id=:device_id;");
    sql_prepare(_updateInfo, "update device set info=:info where id=:device_id;");
    sql_prepare(_updateReg, "update device set reg_first=:reg_first where id=:device_id;");
    sql_prepare(_updateShow, "update device set show_on_map=:show_on_map where id=:device_id;");
    sql_prepare(_updateLog, "update device set log=:log where id=:device_id;");
    sql_prepare(_deleteFromChannel, "delete from device_channel where device_id = :device_id;");
    sql_prepare(_deleteFromDevice, "delete from device where id=:device_id;");
}

caf::result<mccmsg::device::List_ResponsePtr> Device::execute(const mccmsg::device::List_Request& request)
{
    auto r = getList();
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::device::List_Response>(&request, r.take());
}

caf::result<mccmsg::device::Description_ResponsePtr> Device::execute(const mccmsg::device::Description_Request& request)
{
    auto r = getOne(request.data());
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::device::Description_Response>(&request, r.take());
}

caf::result<mccmsg::device::DescriptionS_ResponsePtr> Device::execute(const mccmsg::device::DescriptionS_Request& request)
{
    return mccmsg::Error::NotImplemented;
}

caf::result<mccmsg::device::DescriptionList_ResponsePtr> Device::execute(const mccmsg::device::DescriptionList_Request& request)
{
    auto r = getAll();
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::device::DescriptionList_Response>(&request, r.take());
}

caf::result<mccmsg::device::Register_ResponsePtr> Device::execute(const mccmsg::device::Register_Request& request)
{
    ObjectId id;
    auto r = insert(request.data(), id);
    if (r.isErr())
        return r.takeErr();
    auto d = getOne(id);
    if (d.isErr())
        return d.takeErr();
    registered(r.unwrap(), true);
    return mccmsg::make<mccmsg::device::Register_Response>(&request, d.take());
}

caf::result<mccmsg::device::UnRegister_ResponsePtr> Device::execute(const mccmsg::device::UnRegister_Request& request)
{
    mccmsg::Device device = request.data();
    auto r = getId(device);
    if (r.isNone())
        return mccmsg::make_error(mccmsg::Error::DeviceUnknown);

    auto cs = getChannels(r.unwrap());

    ObjectId device_id = r.unwrap();
    sqlite3pp::transaction transaction(_db->db(), false);

    _deleteFromChannel.reset();
    print(binds(&_deleteFromChannel, ":device_id", device_id));
    print(exec(&_deleteFromChannel));

    _deleteFromDevice.reset();
    print(binds(&_deleteFromDevice, ":device_id", device_id));
    print(exec(&_deleteFromDevice));
    transaction.commit();

    for (const auto& i : cs)
        _db->channel().updated(i);

    registered(device, false);
    return mccmsg::make<mccmsg::device::UnRegister_Response>(&request);
}

bmcl::Option<mccmsg::DeviceDescription> Device::get(const sqlite3pp::selecter::row& r)
{
    mccmsg::Device device_name = mccmsg::Device(bmcl::Uuid::createFromString(r.get<bmcl::StringView>("name")).unwrapOption().unwrapOr(bmcl::Uuid::createNil()));
    bmcl::StringView device_info = r.get<bmcl::StringView>("info");
    bmcl::StringView device_settings = r.get<bmcl::StringView>("settings");
    //ObjectId protocol_id = r.get<int64_t>("protocol_id");
    mccmsg::DeviceId protocol_value = r.get<int64_t>("protocol_value");

    bool device_reg = r.get<bool>("reg_first");
    bool device_map = r.get<bool>("show_on_map");
    bool log = r.get<bool>("log");
    bmcl::SharedBytes pixmap = bmcl::SharedBytes::create(r.get<bmcl::Bytes>("device_pixmap"));
    bmcl::Option<mccmsg::DeviceUi> deviceUi = _db->deviceUi().getName(r.get<int64_t>("device_ui_id"));
    bmcl::Option<mccmsg::Firmware> firmware = _db->firmware().getName(r.get<int64_t>("firmware_id"));
    bmcl::Option<mccmsg::Protocol> protocol = _db->protocol().getName(r.get<int64_t>("protocol_id"));
    mccmsg::ProtocolId protocolId(device_name, protocol.unwrapOr(mccmsg::Protocol()), protocol_value);
    return bmcl::makeRc<const mccmsg::DeviceDescriptionObj>(device_name, device_info, device_settings, protocolId, deviceUi, pixmap, firmware, device_reg, device_map, log);
}

bmcl::Result<mccmsg::Device, caf::error> Device::insert(const mccmsg::DeviceDescription& d, ObjectId& id)
{
    auto protocol = _db->protocol().getId(d->protocolId().protocol());
    if (protocol.isNone())
        return mccmsg::make_error(mccmsg::Error::ProtocolUnknown);

    mccmsg::Device name = mccmsg::Device::generate();

    _queryReg.reset();
    print(binds(&_queryReg, ":name", name.toStdString(), sqlite3pp::copy));
    print(binds(&_queryReg, ":info", d->info(), sqlite3pp::nocopy));
    print(binds(&_queryReg, ":protocol_id", protocol.unwrap()));
    print(binds(&_queryReg, ":protocol_value", (int64_t)d->protocolId().id()));
    print(binds(&_queryReg, ":device_pixmap", d->pixmap().view(), sqlite3pp::nocopy));
    print(binds(&_queryReg, ":log", d->log()));

    auto r = _queryReg.insert();
    if (r.isErr())
        return mccmsg::make_error(mccmsg::Error::CantRegister, fmt::format("{} {}", _queryReg.sql().non_null(), _queryReg.err_msg().unwrapOr("")));
    id = r.unwrap();
    return name;
}

caf::result<mccmsg::device::Connect_ResponsePtr> Device::execute(const mccmsg::device::Connect_Request& request)
{
    if (!request.data()._isConnect)
        return executeDisconnect(request);
    else
        return executeConnect(request);
}

bmcl::Option<caf::error> Device::connect(const mccmsg::Device& device, const mccmsg::Channel& channel)
{
    ObjectId device_id;
    auto d = getOne(device, &device_id);
    if (d.isErr())
        return mccmsg::make_error(mccmsg::Error::DeviceUnknown);
    ObjectId channel_id;
    auto c = _db->channel().getOne(channel, &channel_id);
    if (c.isErr())
        return mccmsg::make_error(mccmsg::Error::ChannelUnknown);
    ObjectId protocol_id;
    auto p = _db->protocol().getOne(c.unwrap()->protocol(), &protocol_id);
    if (p.isErr())
        return mccmsg::make_error(mccmsg::Error::ProtocolUnknown);

    if (d.unwrap()->protocolId().protocol() != c.unwrap()->protocol())
        return mccmsg::make_error(mccmsg::Error::ProtocolsShouldBeSame);
    if (!p.unwrap()->shareable() && !c.unwrap()->connectedDevices().empty())
        return mccmsg::make_error(mccmsg::Error::ChannelCantShare);

    _queryConnect.reset();
    print(binds(&_queryConnect, ":channel_id", channel_id));
    print(binds(&_queryConnect, ":device_id", device_id));
    auto i = print(mccdb::insert(_queryConnect));
    if (i.isErr())
        return mccmsg::make_error(mccmsg::Error::CantJoin, "unknown error: " + i.takeErr());
    return bmcl::None;
}

caf::result<mccmsg::device::Connect_ResponsePtr> Device::executeConnect(const mccmsg::device::Connect_Request& request)
{
    auto r = connect(request.data()._device, request.data()._channel);
    if (r.isSome())
        return r.take();

    _db->deviceConnected(request.data()._isConnect, request.data()._device, request.data()._channel);
    _db->channel().updated(request.data()._channel);
    updated(request.data()._device);
    return mccmsg::make<mccmsg::device::Connect_Response>(&request);
}

caf::result<mccmsg::device::Connect_ResponsePtr> Device::executeDisconnect(const mccmsg::device::Connect_Request& request)
{
    ObjectId device_id;
    auto d = getOne(request.data()._device, &device_id);
    if (d.isErr())
        return mccmsg::make_error(mccmsg::Error::DeviceUnknown);
    ObjectId channel_id;
    auto c = _db->channel().getOne(request.data()._channel, &channel_id);
    if (c.isErr())
        return mccmsg::make_error(mccmsg::Error::ChannelUnknown);

    _queryDisconnect.reset();
    print(binds(&_queryDisconnect, ":channel_id", channel_id));
    print(binds(&_queryDisconnect, ":device_id", device_id));

    print(exec(&_queryDisconnect));
    _db->deviceConnected(request.data()._isConnect, request.data()._device, request.data()._channel);
    _db->channel().updated(request.data()._channel);
    updated(request.data()._device);
    return mccmsg::make<mccmsg::device::Connect_Response>(&request);
}

mccmsg::Channels Device::getChannels(int64_t device_id)
{
    _queryChannels.reset();
    print(binds(&_queryChannels, ":device_id", device_id));
    print(exec(&_queryChannels));

    mccmsg::Channels cs;
    while (_queryChannels.next())
    {
        auto r = _queryChannels.get_row();
        cs.emplace_back(mccmsg::Channel(bmcl::Uuid::createFromString(r.get<bmcl::StringView>("channel_name")).unwrapOption().unwrapOr(bmcl::Uuid::createNil())));
    }
    return cs;
}

}
}
