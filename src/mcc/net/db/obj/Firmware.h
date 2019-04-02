#pragma once
#include "mcc/Rc.h"
#include "mcc/msg/ptr/Firmware.h"
#include "mcc/net/db/Sql.h"
#include "mcc/net/db/obj/Object.h"
#include "mcc/net/NetPlugin.h"
#include "mcc/plugin/Fwd.h"

namespace mccmsg { class ProtocolController; }

namespace mccdb {
namespace dbobj {

class DbObjInternal;

class Firmware : public QueryObject<mccmsg::FirmwareDescription, mccmsg::Firmware>
{
public:
    explicit Firmware(DbObjInternal*);
    ~Firmware();
    void updatePlugins(const mccmsg::ProtocolController*);

    caf::result<mccmsg::firmware::Update_ResponsePtr> execute(const mccmsg::firmware::Update_Request& request);
    caf::result<mccmsg::firmware::List_ResponsePtr> execute(const mccmsg::firmware::List_Request& request);
    caf::result<mccmsg::firmware::DescriptionList_ResponsePtr> execute(const mccmsg::firmware::DescriptionList_Request& request);
    caf::result<mccmsg::firmware::Description_ResponsePtr> execute(const mccmsg::firmware::Description_Request& request);
    caf::result<mccmsg::firmware::DescriptionS_ResponsePtr> execute(const mccmsg::firmware::DescriptionS_Request& request);
    caf::result<mccmsg::firmware::Register_ResponsePtr> execute(const mccmsg::firmware::Register_Request& request);
    caf::result<mccmsg::firmware::UnRegister_ResponsePtr> execute(const mccmsg::firmware::UnRegister_Request& request);
    mccmsg::FirmwareDescription getbyLocal(const mccmsg::ProtocolValue&);

    bmcl::Result<mccmsg::Firmware, caf::error> insert(const mccmsg::FirmwareDescription& d, ObjectId& id);
private:
    bmcl::Option<mccmsg::FirmwareDescription> get(const sqlite3pp::selecter::row& r);

    bmcl::OptionRc<const mccmsg::IFirmware> loadFrm(const mccmsg::ProtocolValue& id, bmcl::Bytes bytes);

    sqlite3pp::selecter _firmware_by_local;
    sqlite3pp::selecter _devices_by_firmware;
    sqlite3pp::statement _clean_firmware;
    sqlite3pp::statement _delete_firmware;
    bmcl::Rc<const mccmsg::ProtocolController> _pc;
};

}
}
