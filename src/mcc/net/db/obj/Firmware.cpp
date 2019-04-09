#include <caf/make_message.hpp>
#include <caf/result.hpp>
#include <bmcl/Buffer.h>
#include <bmcl/Bytes.h>
#include <bmcl/OptionRc.h>
#include <bmcl/MakeRc.h>
#include "mcc/msg/ProtocolController.h"
#include "mcc/msg/ptr/Firmware.h"
#include "mcc/net/NetPlugin.h"
#include "mcc/net/db/Sql.h"
#include "mcc/net/db/obj/Firmware.h"
#include "mcc/net/db/obj/Device.h"
#include "mcc/net/db/obj/Protocol.h"
#include "mcc/net/db/DbHandler.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::firmware::List_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::firmware::UnRegister_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::firmware::DescriptionList_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::firmware::Description_ResponsePtr);

namespace mccdb {
namespace dbobj {

Firmware::~Firmware(){}
Firmware::Firmware(DbObjInternal* db)
    : QueryObject(db, "firmware", "binary, protocol_id"
                    , std::bind(&Firmware::get, this, std::placeholders::_1)
                    , std::bind(&Firmware::insert, this, std::placeholders::_1, std::placeholders::_2))
    , _firmware_by_local(db->db())
    , _devices_by_firmware(db->db())
    , _clean_firmware(db->db())
    , _delete_firmware(db->db())
{
    sql_prepare(_firmware_by_local, "select id from firmware where info=:info and protocol_id = (select id from protocol where name=:protocol);");
    sql_prepare(_devices_by_firmware, "select name from device where firmware_id = (select id from firmware where name=:firmware)");
    sql_prepare(_clean_firmware, "update device set firmware_id = null where firmware_id = (select id from firmware where name=:firmware)");
    sql_prepare(_delete_firmware, "delete from firmware where name = :firmware");
}

caf::result<mccmsg::firmware::Update_ResponsePtr> Firmware::execute(const mccmsg::firmware::Update_Request&)
{
    return mccmsg::make_error(mccmsg::Error::NotImplemented);
}

caf::result<mccmsg::firmware::List_ResponsePtr> Firmware::execute(const mccmsg::firmware::List_Request& request)
{
    auto r = getList();
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::firmware::List_Response>(&request, r.take());
}

caf::result<mccmsg::firmware::Description_ResponsePtr> Firmware::execute(const mccmsg::firmware::Description_Request& request)
{
    auto r = getOne(request.data());
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::firmware::Description_Response>(&request, r.take());
}

caf::result<mccmsg::firmware::DescriptionList_ResponsePtr> Firmware::execute(const mccmsg::firmware::DescriptionList_Request& request)
{
    auto r = getAll();
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::firmware::DescriptionList_Response>(&request, r.take());
}

caf::result<mccmsg::firmware::DescriptionS_ResponsePtr> Firmware::execute(const mccmsg::firmware::DescriptionS_Request& request)
{
    auto p = getbyLocal(request.data());
    if (p.isNull())
        return mccmsg::make_error(mccmsg::Error::FirmwareUnknown);
    return mccmsg::make<mccmsg::firmware::Description_Response>(&request, p);
}

caf::result<mccmsg::firmware::UnRegister_ResponsePtr> Firmware::execute(const mccmsg::firmware::UnRegister_Request& request)
{
    _devices_by_firmware.reset();
    print(binds(&_devices_by_firmware, ":firmware", request.data().toStringRepr().view(), sqlite3pp::copy));
    auto r = print(exec(&_devices_by_firmware));
    if (r.isSome())
        return mccmsg::make_error(mccmsg::Error::CantUnRegister, r.take());

    mccmsg::Devices devices;
    while (_devices_by_firmware.next())
    {
        const auto row = _devices_by_firmware.get_row();
        bmcl::Uuid name = bmcl::Uuid::createFromString(row.get<bmcl::StringView>("name")).takeOption().unwrapOr(bmcl::Uuid::createNil());
        devices.emplace_back(mccmsg::Device(name));
    }

    _clean_firmware.reset();
    print(binds(&_clean_firmware, ":firmware", request.data().toStringRepr().view(), sqlite3pp::copy));
    r = print(exec(&_clean_firmware));
    assert(devices.size() == (std::size_t)_db->db().changes().unwrapOr(0));
    if (r.isSome())
        return mccmsg::make_error(mccmsg::Error::CantUnRegister, r.take());

    for (const auto& i : devices)
        _db->device().updated(i);

    _delete_firmware.reset();
    print(binds(&_delete_firmware, ":firmware", request.data().toStringRepr().view(), sqlite3pp::copy));
    r = print(exec(&_delete_firmware));
    if (r.isSome())
        return mccmsg::make_error(mccmsg::Error::CantUnRegister, r.take());

    registered(request.data(), false);
    return mccmsg::make<mccmsg::firmware::UnRegister_Response>(&request);
}

bool isSame(bmcl::Bytes l, bmcl::Bytes r)
{
    if (l.size() != r.size())
        return false;
    return memcmp(l.data(), r.data(), r.size()) == 0;
}

caf::result<mccmsg::firmware::Register_ResponsePtr> Firmware::execute(const mccmsg::firmware::Register_Request& request)
{
    ObjectId id;
    auto r = insert(request.data(), id);
    if (r.isErr())
        return r.takeErr();
    auto d = getOne(id);
    if (d.isErr())
        return d.takeErr();
    registered(r.unwrap(), true);
    return mccmsg::make<mccmsg::firmware::Register_Response>(&request, d.take());
}

mccmsg::FirmwareDescription Firmware::getbyLocal(const mccmsg::ProtocolValue& pv)
{
    _firmware_by_local.reset();
    print(binds(&_firmware_by_local, ":info", bmcl::StringView(pv.value()), sqlite3pp::nocopy));
    print(binds(&_firmware_by_local, ":protocol", pv.protocol().toStringRepr().view(), sqlite3pp::copy));
    auto r = print(exec(&_firmware_by_local));
    if (r.isSome() || !_firmware_by_local.next())
        return nullptr;
    auto row = _firmware_by_local.get_row();

    auto d = getOne(row.get<int64_t>("id"));
    return d.unwrapOr(nullptr);
}

void Firmware::updatePlugins(const mccmsg::ProtocolController* p)
{
    _pc = p;
}
bmcl::OptionRc<const mccmsg::IFirmware> Firmware::loadFrm(const mccmsg::ProtocolValue& id, bmcl::Bytes bytes)
{
    auto r = _pc->decodeFirmware(id, bytes);
    if(r.isNone())
    {
        BMCL_WARNING() << fmt::format("Прошивка для неизвестного протокола {}", id.protocol().toStdString());
        assert(false);
        return bmcl::None;
    }
    return r;
}

bmcl::Option<mccmsg::FirmwareDescription> Firmware::get(const sqlite3pp::selecter::row& r)
{
    bmcl::Uuid n = bmcl::Uuid::createFromString(r.get<bmcl::StringView>("name")).takeOption().unwrapOr(bmcl::Uuid::createNil());

    mccmsg::Firmware name = mccmsg::Firmware(n);
    int64_t protocol_id = r.get<int64_t>("protocol_id");
    bmcl::Bytes bs = r.get<bmcl::Bytes>("binary");
    bmcl::StringView info = r.get<bmcl::StringView>("info");

    auto p = _db->protocol().getOne(protocol_id);
    if (p.isErr())
        return bmcl::None;

    mccmsg::ProtocolValue pv(p.unwrap()->name(), info.toStdString());
    auto f = loadFrm(pv, bs);
    if (f.isNone())
        return bmcl::None;
    return bmcl::makeRc<const mccmsg::FirmwareDescriptionObj>(name, pv, f.take());
}

bmcl::Result<mccmsg::Firmware, caf::error> Firmware::insert(const mccmsg::FirmwareDescription& d, ObjectId& id)
{
    const auto& f = d->frm();
    if (f.isNull())
        return mccmsg::make_error(mccmsg::Error::CantRegister, "Прошивка не задана");

    auto p = getbyLocal(d->id());
    if (!p.isNull())
    {
        if (!isSame(d->frm()->encode(), p->frm()->encode()))
        {
            BMCL_WARNING() << fmt::format("Прошивка {} {} уже зарегистрирована, но отличается от вновь регистрируемой с тем же именем! Будет использована старая!", d->id().protocol().toStdString(), d->id().value());
        }
        auto tmp = getId(p->name());
        assert(tmp.isSome());
        id = tmp.unwrapOr(0);
        return p->name();
    }

    mccmsg::Firmware name = mccmsg::Firmware::generate();
    bmcl::Buffer buffer = f->encode();

    auto protocol_id = _db->protocol().getId(d->id().protocol());
    if (protocol_id.isNone())
        return mccmsg::make_error(mccmsg::Error::ProtocolUnknown);

    _queryReg.reset();
    print(binds(&_queryReg, ":name", name.toStringRepr().view(), sqlite3pp::copy));
    print(binds(&_queryReg, ":info", bmcl::StringView(d->id().value()), sqlite3pp::nocopy));
    print(binds(&_queryReg, ":protocol_id", protocol_id.unwrap()));
    print(binds(&_queryReg, ":binary", buffer.asBytes(), sqlite3pp::nocopy));

    auto r = _queryReg.insert();
    if (r.isErr())
        return mccmsg::make_error(mccmsg::Error::CantRegister, fmt::format("{} {}", _queryReg.sql().non_null(), _queryReg.err_msg().unwrapOr("")));
    id = r.unwrap();
    return name;
}

}
}
