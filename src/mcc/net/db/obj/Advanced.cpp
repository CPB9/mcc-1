#include <caf/make_message.hpp>
#include <caf/result.hpp>
#include "mcc/msg/ptr/Advanced.h"
#include "mcc/net/db/Sql.h"
#include "mcc/net/db/obj/Channel.h"
#include "mcc/net/db/obj/Device.h"
#include "mcc/net/db/obj/Advanced.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::advanced::ChannelAndDeviceRegister_ResponsePtr)

namespace mccdb {
namespace dbobj {

Advanced::Advanced(DbObjInternal* db) : _db(db)
{
}

caf::result<mccmsg::advanced::ChannelAndDeviceRegister_ResponsePtr> Advanced::execute(const mccmsg::advanced::ChannelAndDeviceRegister_Request& request)
{
    const mccmsg::DeviceDescription& d = request.data().device;
    const mccmsg::ChannelDescription& c = request.data().channel;

    sqlite3pp::transaction t(_db->db(), false);

    mccmsg::Device deviceName = d->name();
    if (deviceName.isNil())
    {
        ObjectId id;
        auto r = _db->device().insert(d, id);
        if (r.isErr())
            return r.takeErr();
        deviceName = r.take();
    }

    mccmsg::Channel channelName = c->name();
    if (channelName.isNil())
    {
        ObjectId id;
        auto r = _db->channel().insert(c, id);
        if (r.isErr())
            return r.takeErr();
        channelName = r.take();
    }

    auto r = _db->device().connect(deviceName, channelName);
    if (r.isSome())
        r.take();

    t.commit();
    if (d->name().isNil())
        _db->device().registered(deviceName, true);
    if (c->name().isNil())
        _db->channel().registered(channelName, true);

    _db->deviceConnected(true, d->name(), c->name());
    _db->channel().updated(channelName);
    _db->device().updated(deviceName);
    return mccmsg::make<mccmsg::advanced::ChannelAndDeviceRegister_Response>(&request, mccmsg::ProtocolId(deviceName, d->protocolId().protocol(), d->protocolId().id()), channelName);
}

}
}
