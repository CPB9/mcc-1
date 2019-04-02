#pragma once
#include "mcc/msg/ptr/Fwd.h"
#include "mcc/msg/obj/Radar.h"
#include "mcc/net/db/Sql.h"
#include "mcc/net/db/DbHandler.h"
#include "mcc/net/db/obj/Object.h"

namespace mccdb {
namespace dbobj {

class Radar : public QueryObject<mccmsg::RadarDescription, mccmsg::Radar>
{
public:
    Radar(DbObjInternal*);
    caf::result<mccmsg::radar::Register_ResponsePtr> execute(const mccmsg::radar::Register_Request& request);
    caf::result<mccmsg::radar::UnRegister_ResponsePtr> execute(const mccmsg::radar::UnRegister_Request& request);
    caf::result<mccmsg::radar::Update_ResponsePtr> execute(const mccmsg::radar::Update_Request& request);
    caf::result<mccmsg::radar::List_ResponsePtr> execute(const mccmsg::radar::List_Request& request);
    caf::result<mccmsg::radar::Description_ResponsePtr> execute(const mccmsg::radar::Description_Request& request);
    caf::result<mccmsg::radar::DescriptionS_ResponsePtr> execute(const mccmsg::radar::DescriptionS_Request& request);
    caf::result<mccmsg::radar::DescriptionList_ResponsePtr> execute(const mccmsg::radar::DescriptionList_Request& request);
    bmcl::Result<mccmsg::Radar, caf::error> insert(const mccmsg::RadarDescription& r, ObjectId& id);
private:
    static bmcl::Option<mccmsg::RadarDescription> get(const sqlite3pp::selecter::row& r);

    sqlite3pp::statement _unregister;
    sqlite3pp::statement _update;
};

}
}
