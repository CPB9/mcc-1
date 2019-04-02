#include <caf/make_message.hpp>
#include <caf/result.hpp>
#include "mcc/msg/ptr/Channel.h"
#include "mcc/net/db/Sql.h"
#include "mcc/net/db/obj/Channel.h"
#include "mcc/net/db/obj/Device.h"
#include "mcc/net/db/obj/Radar.h"
#include "mcc/net/db/obj/Protocol.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::channel::List_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::channel::UnRegister_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::channel::DescriptionList_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::channel::Description_ResponsePtr);

namespace mccdb {
namespace dbobj {

Channel::Channel(DbObjInternal* db)
    : QueryObject(db, "channel", "log, isDynTimeout, isReadOnly, reconnectTimeout, timeout, settings, protocol_id, radar_id"
                , std::bind(&Channel::get, this, std::placeholders::_1)
                , std::bind(&Channel::insert, this, std::placeholders::_1, std::placeholders::_2)
                )
    , _queryUpdate(db->db())
    , _queryDeleteChannelDevices(db->db())
    , _queryDeleteChannel(db->db())
    , _queryConnectedDevices(db->db())
{
    sql_prepare(_queryUpdate, "update channel set info         = :info          \
                                                , log          = :log           \
                                                , isDynTimeout = :isDynTimeout  \
                                                , isReadOnly   = :isReadOnly    \
                                                , reconnectTimeout = :reconnectTimeout  \
                                                , timeout      = :timeout       \
                                                , settings     = :settings      \
                                                , protocol_id  = :protocol_id   \
                                                , radar_id     = :radar_id      \
                                            where id=:channel_id;");

    sql_prepare(_queryConnectedDevices, "select device.name, device.protocol_value from device_channel, device  \
                                          where device_channel.channel_id = :channel_id and device_channel.device_id = device.id");
    sql_prepare(_queryDeleteChannel, "delete from channel where id = :channel_id;");
    sql_prepare(_queryDeleteChannelDevices, "delete from device_channel where channel_id=:channel_id;");
}

caf::result<mccmsg::channel::List_ResponsePtr> Channel::execute(const mccmsg::channel::List_Request& request)
{
    auto r = getList();
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::channel::List_Response>(&request, r.take());
}

caf::result<mccmsg::channel::Description_ResponsePtr> Channel::execute(const mccmsg::channel::Description_Request& request)
{
    auto r = getOne(request.data());
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::channel::Description_Response>(&request, r.take());
}

caf::result<mccmsg::channel::DescriptionS_ResponsePtr> Channel::execute(const mccmsg::channel::DescriptionS_Request& request)
{
    return mccmsg::Error::NotImplemented;
}

caf::result<mccmsg::channel::DescriptionList_ResponsePtr> Channel::execute(const mccmsg::channel::DescriptionList_Request& request)
{
    auto r = getAll();
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::channel::DescriptionList_Response>(&request, r.take());
}

caf::result<mccmsg::channel::Register_ResponsePtr> Channel::execute(const mccmsg::channel::Register_Request& request)
{
    ObjectId id;
    auto r = insert(request.data(), id);
    if (r.isErr())
        return r.takeErr();
    auto d = getOne(id);
    if (d.isErr())
        return d.takeErr();
    registered(r.unwrap(), true);
    return mccmsg::make<mccmsg::channel::Register_Response>(&request, d.take());
}

caf::result<mccmsg::channel::Update_ResponsePtr> Channel::execute(const mccmsg::channel::Update_Request& request)
{
    const mccmsg::ChannelDescription& dscr = request.data().dscr();
    auto channel = dscr->name();
    std::string settings = dscr->params()->to_string();

    auto channel_id = getId(channel);
    if (channel_id.isNone())
        return mccmsg::make_error(mccmsg::Error::ChannelUnknown);

    auto protocol_id = _db->protocol().getId(dscr->protocol());
    if (protocol_id.isNone())
        return mccmsg::make_error(mccmsg::Error::ProtocolUnknown);

    bmcl::Option<ObjectId> radar_id;
    if (dscr->radar().isSome())
    {
        radar_id = _db->radar().getId(dscr->radar().unwrap());
        if (radar_id.isNone())
            return mccmsg::make_error(mccmsg::Error::RadarUnknown);
    }

    bmcl::Option<int64_t> reconnectTimeout;
    if (dscr->reconnect().isSome())
        reconnectTimeout = dscr->reconnect()->count();

    _queryUpdate.reset();
    print(binds(&_queryUpdate, ":channel_id", channel_id));
    print(binds(&_queryUpdate, ":info", dscr->info(), sqlite3pp::nocopy));
    print(binds(&_queryUpdate, ":log", dscr->log()));
    print(binds(&_queryUpdate, ":isDynTimeout", dscr->isDynamicTimeout()));
    print(binds(&_queryUpdate, ":isReadOnly", dscr->isReadOnly()));
    print(binds(&_queryUpdate, ":reconnectTimeout", reconnectTimeout));
    print(binds(&_queryUpdate, ":timeout", dscr->timeout().count()));
    print(binds(&_queryUpdate, ":protocol_id", protocol_id.unwrap()));
    print(binds(&_queryUpdate, ":settings", settings.c_str(), sqlite3pp::nocopy));
    if (radar_id.isSome())
        print(binds(&_queryUpdate, ":radar_id", radar_id.unwrap()));

    auto r = print(exec(&_queryUpdate));
    if (r.isSome())
        return mccmsg::make_error(mccmsg::Error::CantUpdate, r.take());

    auto v = updated(dscr->name());
    if (v.isNone())
        return mccmsg::make_error(mccmsg::Error::CantUpdate);

    return mccmsg::make<mccmsg::channel::Update_Response>(&request, v.take());
}

caf::result<mccmsg::channel::UnRegister_ResponsePtr> Channel::execute(const mccmsg::channel::UnRegister_Request& request)
{
    auto id = getId(request.data());
    if (id.isNone())
        return mccmsg::make_error(mccmsg::Error::ChannelUnknown);

    auto cds = getConnectedDevices(id.unwrap(), mccmsg::Protocol());
    for (const auto& i : cds)
        _db->device().updated(i.device());

    _queryDeleteChannelDevices.reset();
    print(binds(&_queryDeleteChannelDevices, ":channel_id", id.unwrap()));
    print(exec(&_queryDeleteChannelDevices));

    _queryDeleteChannel.reset();
    print(binds(&_queryDeleteChannel, ":channel_id", id.unwrap()));
    print(exec(&_queryDeleteChannel));

    registered(request.data(), false);
    return mccmsg::make<mccmsg::channel::UnRegister_Response>(&request);
}

bmcl::Option<mccmsg::ChannelDescription> Channel::get(const sqlite3pp::selecter::row& r)
{
    bmcl::Uuid name = bmcl::Uuid::createFromString(r.get<bmcl::StringView>("name")).takeOption().unwrapOr(bmcl::Uuid::createNil());
    mccmsg::Channel channel(name);

    bmcl::Option<mccmsg::Radar> radar;
    if (!r.is_null("radar_id"))
        radar = _db->radar().getName(r.get<int64_t>("radar_id"));

    bmcl::Option<mccmsg::Protocol> protocol = _db->protocol().getName(r.get<int64_t>("protocol_id"));
    if (protocol.isNone())
        return bmcl::None;

    bmcl::Option<std::chrono::seconds> reconnect;
    if (!r.is_null("reconnectTimeout"))
        reconnect = std::chrono::seconds(r.get<int64_t>("reconnectTimeout"));

    bmcl::Option<mccmsg::ChannelDescription> dscr = mccmsg::from_string
    (
        channel
        , protocol.unwrap()
        , r.get<bmcl::StringView>("info")
        , r.get<bmcl::StringView>("settings")
        , r.get<bool>("log")
        , std::chrono::milliseconds(r.get<int64_t>("timeout"))
        , r.get<bool>("isDynTimeout")
        , r.get<bool>("isReadOnly")
        , reconnect
        , radar
        , getConnectedDevices(r.get<int64_t>("id"), protocol.unwrap())
    );
    return dscr;
}

mccmsg::ProtocolIds Channel::getConnectedDevices(const ObjectId& channel_id, const mccmsg::Protocol& protocol)
{
    _queryConnectedDevices.reset();
    print(binds(&_queryConnectedDevices, ":channel_id", channel_id));
    print(exec(&_queryConnectedDevices));

    mccmsg::ProtocolIds protocolIds;
    while (_queryConnectedDevices.next())
    {
        auto r = _queryConnectedDevices.get_row();
        bmcl::Uuid name = bmcl::Uuid::createFromString(r.get<bmcl::StringView>("name")).takeOption().unwrapOr(bmcl::Uuid::createNil());
        mccmsg::ProtocolId id(mccmsg::Device(name), protocol, r.get<int64_t>("protocol_value"));
        protocolIds.emplace_back(id);
    }
    return protocolIds;
}

bmcl::Result<mccmsg::Channel, caf::error> Channel::insert(const mccmsg::ChannelDescription& d, ObjectId& id)
{
    auto name = mccmsg::Channel::generate();
    std::string settings = d->params()->to_string();

    bmcl::Option<ObjectId> protocol_id = _db->protocol().getId(d->protocol());
    if (protocol_id.isNone())
        return mccmsg::make_error(mccmsg::Error::ProtocolUnknown);

    bmcl::Option<ObjectId> radar_id;
    if (d->radar().isSome())
    {
        radar_id = _db->radar().getId(d->radar().unwrap());
        if (radar_id.isNone())
            return mccmsg::make_error(mccmsg::Error::RadarUnknown);
    }

    bmcl::Option<int64_t> reconnectTimeout;
    if (d->reconnect().isSome())
        reconnectTimeout = d->reconnect()->count();

    _queryReg.reset();
    print(binds(&_queryReg, ":name", name.toStdString(), sqlite3pp::copy));
    print(binds(&_queryReg, ":info", d->info(), sqlite3pp::nocopy));
    print(binds(&_queryReg, ":log", d->log()));
    print(binds(&_queryReg, ":isDynTimeout", d->isDynamicTimeout()));
    print(binds(&_queryReg, ":isReadOnly", d->isReadOnly()));
    print(binds(&_queryReg, ":reconnectTimeout", reconnectTimeout));
    print(binds(&_queryReg, ":timeout", d->timeout().count()));
    print(binds(&_queryReg, ":protocol_id", protocol_id.unwrap()));
    print(binds(&_queryReg, ":settings", settings.c_str(), sqlite3pp::nocopy));
    if (radar_id.isSome())
        print(binds(&_queryReg, ":radar", radar_id.unwrap()));

    auto r = _queryReg.insert();
    if (r.isErr())
        return mccmsg::make_error(mccmsg::Error::CantRegister, fmt::format("{} {}", _queryReg.sql().non_null(), _queryReg.err_msg().unwrapOr("")));
    id = r.take();
    return name;
}

}
}
