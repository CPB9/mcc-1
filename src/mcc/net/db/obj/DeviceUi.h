#pragma once
#include "mcc/msg/ptr/DeviceUi.h"
#include "mcc/net/db/Sql.h"
#include "mcc/net/db/DbHandler.h"
#include "mcc/net/db/obj/Object.h"

namespace mccdb {
namespace dbobj {

class DeviceUi : public QueryObject<mccmsg::DeviceUiDescription, mccmsg::DeviceUi>
{
public:
    DeviceUi(DbObjInternal*);
    caf::result<mccmsg::deviceUi::Register_ResponsePtr> execute(const mccmsg::deviceUi::Register_Request& request);
    caf::result<mccmsg::deviceUi::UnRegister_ResponsePtr> execute(const mccmsg::deviceUi::UnRegister_Request& request);
    caf::result<mccmsg::deviceUi::Update_ResponsePtr> execute(const mccmsg::deviceUi::Update_Request& request);
    caf::result<mccmsg::deviceUi::List_ResponsePtr> execute(const mccmsg::deviceUi::List_Request& request);
    caf::result<mccmsg::deviceUi::Description_ResponsePtr> execute(const mccmsg::deviceUi::Description_Request& request);
    caf::result<mccmsg::deviceUi::DescriptionS_ResponsePtr> execute(const mccmsg::deviceUi::DescriptionS_Request& request);
    caf::result<mccmsg::deviceUi::DescriptionList_ResponsePtr> execute(const mccmsg::deviceUi::DescriptionList_Request& request);
    bmcl::Result<mccmsg::DeviceUi, caf::error> insert(const mccmsg::DeviceUiDescription& d, ObjectId& id);
private:
    bmcl::Option<mccmsg::DeviceUiDescription> get(const sqlite3pp::selecter::row& r);
    sqlite3pp::selecter _get_by_local;
};

}
}
