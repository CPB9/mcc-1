#pragma once
#include "mcc/msg/Fwd.h"
#include "mcc/msg/ptr/Protocol.h"
#include "mcc/net/db/Sql.h"
#include "mcc/net/db/obj/Object.h"
#include "mcc/net/db/DbHandler.h"
#include "mcc/plugin/Fwd.h"

namespace mccdb {
namespace dbobj {


class Protocol : public QueryObject<mccmsg::ProtocolDescription, mccmsg::Protocol>
{
public:
    Protocol(DbObjInternal*);
    void updatePlugins(const mccmsg::ProtocolController*);

    caf::result<mccmsg::protocol::Update_ResponsePtr> execute(const mccmsg::protocol::Update_Request& request);
    caf::result<mccmsg::protocol::Register_ResponsePtr> execute(const mccmsg::protocol::Register_Request& request);
    caf::result<mccmsg::protocol::UnRegister_ResponsePtr> execute(const mccmsg::protocol::UnRegister_Request& request);
    caf::result<mccmsg::protocol::List_ResponsePtr> execute(const mccmsg::protocol::List_Request& request);
    caf::result<mccmsg::protocol::Description_ResponsePtr> execute(const mccmsg::protocol::Description_Request& request);
    caf::result<mccmsg::protocol::DescriptionS_ResponsePtr> execute(const mccmsg::protocol::DescriptionS_Request& request);
    caf::result<mccmsg::protocol::DescriptionList_ResponsePtr> execute(const mccmsg::protocol::DescriptionList_Request& request);

    bmcl::Result<mccmsg::Protocol, caf::error> insert(const mccmsg::ProtocolDescription& d, ObjectId& id);
private:
    bmcl::Option<mccmsg::ProtocolDescription> get(const sqlite3pp::selecter::row& r);

    mccmsg::ProtocolDescriptions _protocols;
};




}
}
