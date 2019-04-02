#pragma once
#include "mcc/msg/ptr/Channel.h"
#include "mcc/net/db/Sql.h"
#include "mcc/net/db/obj/Object.h"
#include "mcc/net/db/DbHandler.h"

namespace mccdb {
namespace dbobj {

class Channel : public QueryObject<mccmsg::ChannelDescription, mccmsg::Channel>
{
public:
    Channel(DbObjInternal*);
    caf::result<mccmsg::channel::List_ResponsePtr> execute(const mccmsg::channel::List_Request& request);
    caf::result<mccmsg::channel::Description_ResponsePtr> execute(const mccmsg::channel::Description_Request& request);
    caf::result<mccmsg::channel::DescriptionS_ResponsePtr> execute(const mccmsg::channel::DescriptionS_Request& request);
    caf::result<mccmsg::channel::DescriptionList_ResponsePtr> execute(const mccmsg::channel::DescriptionList_Request& request);
    caf::result<mccmsg::channel::Register_ResponsePtr> execute(const mccmsg::channel::Register_Request& request);
    caf::result<mccmsg::channel::Update_ResponsePtr> execute(const mccmsg::channel::Update_Request& request);
    caf::result<mccmsg::channel::UnRegister_ResponsePtr> execute(const mccmsg::channel::UnRegister_Request& request);

    bmcl::Result<mccmsg::Channel, caf::error> insert(const mccmsg::ChannelDescription& d, ObjectId& id);
private:
    bmcl::Option<mccmsg::ChannelDescription> get(const sqlite3pp::selecter::row& r);

    mccmsg::ProtocolIds getConnectedDevices(const ObjectId& channel_id, const mccmsg::Protocol& protocol);

    sqlite3pp::inserter _queryUpdate;
    sqlite3pp::statement _queryDeleteChannelDevices;
    sqlite3pp::statement _queryDeleteChannel;
    sqlite3pp::selecter _queryConnectedDevices;
};

}
}
