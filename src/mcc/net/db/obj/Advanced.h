#pragma once
#include "mcc/msg/ptr/Advanced.h"
#include "mcc/net/db/Sql.h"
#include "mcc/net/db/obj/Object.h"
#include "mcc/net/db/DbHandler.h"

namespace mccdb {
namespace dbobj {

class Advanced
{
public:
    Advanced(DbObjInternal*);
    caf::result<mccmsg::advanced::ChannelAndDeviceRegister_ResponsePtr> execute(const mccmsg::advanced::ChannelAndDeviceRegister_Request& request);

private:
    DbObjInternal* _db;
};

}
}
