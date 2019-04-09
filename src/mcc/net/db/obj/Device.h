#pragma once
#include <caf/error.hpp>
#include "mcc/msg/FwdExt.h"
#include "mcc/msg/obj/Device.h"
#include "mcc/msg/ptr/Fwd.h"
#include "mcc/net/db/Sql.h"
#include "mcc/net/db/DbHandler.h"
#include "mcc/net/db/obj/Object.h"

namespace mccdb {
namespace dbobj {

class Device : public QueryObject<mccmsg::DeviceDescription, mccmsg::Device>
{
public:
    Device(DbObjInternal*);
    caf::result<mccmsg::device::List_ResponsePtr> execute(const mccmsg::device::List_Request& request);
    caf::result<mccmsg::device::Register_ResponsePtr> execute(const mccmsg::device::Register_Request& request);
    caf::result<mccmsg::device::UnRegister_ResponsePtr> execute(const mccmsg::device::UnRegister_Request& request);
    caf::result<mccmsg::device::Description_ResponsePtr> execute(const mccmsg::device::Description_Request& request);
    caf::result<mccmsg::device::DescriptionS_ResponsePtr> execute(const mccmsg::device::DescriptionS_Request& request);
    caf::result<mccmsg::device::DescriptionList_ResponsePtr> execute(const mccmsg::device::DescriptionList_Request& request);
    caf::result<mccmsg::device::Connect_ResponsePtr> execute(const mccmsg::device::Connect_Request& request);
    caf::result<mccmsg::device::Update_ResponsePtr> execute(const mccmsg::device::Update_Request& request);

    bmcl::Result<mccmsg::Device, caf::error> insert(const mccmsg::DeviceDescription& d, ObjectId& id);
    bmcl::Option<caf::error> connect(const mccmsg::Device& device, const mccmsg::Channel& channel);
private:
    bmcl::Option<mccmsg::DeviceDescription> get(const sqlite3pp::selecter::row& r);

    mccmsg::Channels getChannels(int64_t);

    caf::result<mccmsg::device::Connect_ResponsePtr> executeConnect(const mccmsg::device::Connect_Request& request);
    caf::result<mccmsg::device::Connect_ResponsePtr> executeDisconnect(const mccmsg::device::Connect_Request& request);
    bmcl::Result<bool, std::string> updateFirmware(ObjectId device_id, const mccmsg::DeviceDescription&, const bmcl::Option<mccmsg::Firmware>& firmware);
    bmcl::Result<bool, std::string> updateUi(ObjectId device_id, const mccmsg::DeviceDescription&, const bmcl::Option<mccmsg::DeviceUi>& ui);
    bmcl::Result<bool, std::string> updatePixmap(ObjectId device_id, const mccmsg::DeviceDescription&, const bmcl::SharedBytes& kind);
    bmcl::Result<bool, std::string> updateSettings(ObjectId device_id, const mccmsg::DeviceDescription&, const std::string& settings);
    bmcl::Result<bool, std::string> updateInfo(ObjectId device_id, const mccmsg::DeviceDescription&, const std::string& info);
    bmcl::Result<bool, std::string> updateReg(ObjectId device_id, const mccmsg::DeviceDescription&, const bool& reg_first);
    bmcl::Result<bool, std::string> updateLog(ObjectId device_id, const mccmsg::DeviceDescription& old, const bool& log);

    sqlite3pp::statement _queryDisconnect;
    sqlite3pp::inserter _queryConnect;
    sqlite3pp::selecter _queryChannels;

    sqlite3pp::statement _updateUi;
    sqlite3pp::statement _updatePixmap;
    sqlite3pp::statement _updateSettings;
    sqlite3pp::statement _updateInfo;
    sqlite3pp::statement _updateFirmware;
    sqlite3pp::statement _updateReg;
    sqlite3pp::statement _updateLog;

    sqlite3pp::statement _deleteFromChannel;
    sqlite3pp::statement _deleteFromDevice;
};

}
}
