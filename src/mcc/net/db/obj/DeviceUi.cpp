#include <caf/make_message.hpp>
#include <caf/result.hpp>
#include <bmcl/MakeRc.h>
#include "mcc/net/db/Sql.h"
#include "mcc/net/db/obj/Protocol.h"
#include "mcc/net/db/obj/DeviceUi.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::deviceUi::List_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::deviceUi::UnRegister_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::deviceUi::DescriptionList_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::deviceUi::Description_ResponsePtr);

namespace mccdb {
namespace dbobj {

DeviceUi::DeviceUi(DbObjInternal* db)
    : QueryObject(db
                , "device_ui"
                , "protocol_id, binary"
                , std::bind(&DeviceUi::get, this, std::placeholders::_1)
                , std::bind(&DeviceUi::insert, this, std::placeholders::_1, std::placeholders::_2))
                , _get_by_local(db->db())
{
    sql_prepare(_get_by_local, "select id from device_ui where info=:info and protocol_id = (select id from protocol where name=:protocol);");
}

caf::result<mccmsg::deviceUi::Register_ResponsePtr> DeviceUi::execute(const mccmsg::deviceUi::Register_Request& request)
{
    ObjectId id;
    auto r = insert(request.data(), id);
    if (r.isErr())
        return r.takeErr();
    auto d = getOne(id);
    if (d.isErr())
        return d.takeErr();
    registered(r.unwrap(), true);
    return mccmsg::make<mccmsg::deviceUi::Register_Response>(&request, d.take());
}

caf::result<mccmsg::deviceUi::UnRegister_ResponsePtr> DeviceUi::execute(const mccmsg::deviceUi::UnRegister_Request&)
{
    return mccmsg::make_error(mccmsg::Error::NotImplemented);
}

caf::result<mccmsg::deviceUi::Update_ResponsePtr> DeviceUi::execute(const mccmsg::deviceUi::Update_Request&)
{
    return mccmsg::make_error(mccmsg::Error::NotImplemented);
}

caf::result<mccmsg::deviceUi::List_ResponsePtr> DeviceUi::execute(const mccmsg::deviceUi::List_Request& request)
{
    auto r = getList();
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::deviceUi::List_Response>(&request, r.take());
}

caf::result<mccmsg::deviceUi::Description_ResponsePtr> DeviceUi::execute(const mccmsg::deviceUi::Description_Request& request)
{
    auto r = getOne(request.data());
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::deviceUi::Description_Response>(&request, r.take());

}

caf::result<mccmsg::deviceUi::DescriptionS_ResponsePtr> DeviceUi::execute(const mccmsg::deviceUi::DescriptionS_Request& request)
{
    const mccmsg::ProtocolValue& pv = request.data();
    _get_by_local.reset();
    print(binds(&_get_by_local, ":protocol", pv.protocol().toStringRepr().view(), sqlite3pp::copy));
    print(binds(&_get_by_local, ":info", bmcl::StringView(pv.value()), sqlite3pp::copy));
    auto r = print(exec(&_get_by_local));
    if (r.isSome() || !_get_by_local.next())
        return mccmsg::Error::NotFound;
    auto row = _get_by_local.get_row();
    auto id = row.get<int64_t>("id");
    auto p = getOne(id);
    if (p.isErr())
        return mccmsg::Error::NotFound;
    return mccmsg::make<mccmsg::deviceUi::DescriptionS_Response>(&request, p.take());
}

caf::result<mccmsg::deviceUi::DescriptionList_ResponsePtr> DeviceUi::execute(const mccmsg::deviceUi::DescriptionList_Request& request)
{
    auto r = getAll();
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::deviceUi::DescriptionList_Response>(&request, r.take());
}

bmcl::Option<mccmsg::DeviceUiDescription> DeviceUi::get(const sqlite3pp::selecter::row& r)
{
    bmcl::Bytes uiBytes = r.get<bmcl::Bytes>("binary");
    bmcl::Uuid name = bmcl::Uuid::createFromString(r.get<bmcl::StringView>("name")).takeOption().unwrapOr(bmcl::Uuid::createNil());
    int64_t protocol_id = r.get<int64_t>("protocol_id");
    auto p = _db->protocol().getOne(protocol_id);
    if (p.isErr())
        return bmcl::None;
    mccmsg::ProtocolValue pv(p.unwrap()->name(), r.get<bmcl::StringView>("info").toStdString());
    return bmcl::makeRc<const mccmsg::DeviceUiDescriptionObj>(mccmsg::DeviceUi(name), std::move(pv) , bmcl::SharedBytes::create(uiBytes));
}

bmcl::Result<mccmsg::DeviceUi, caf::error> DeviceUi::insert(const mccmsg::DeviceUiDescription& d, ObjectId& id)
{
    const auto& f = d->data();
    if (f.isNull())
        return mccmsg::make_error(mccmsg::Error::CantRegister, "Ui не задан");

//     auto p = getbyLocal(mccmsg::ProtocolValue(d.protocol(), f->name()));
//     if (p.isSome())
//     {
//         if (!isSame(d.frm()->encode(), p->frm()->encode()))
//         {
//             BMCL_WARNING() << fmt::format("Прошивка {} {} уже зарегистрирована, но отличается от вновь регистрируемой с тем же именем! Будет использована старая!", d.protocol().toStdString(), f->name());
//         }
//         auto tmp = getId(p->name());
//         assert(tmp.isSome());
//         id = tmp.unwrapOr(0);
//         return p->name();
//     }

    mccmsg::DeviceUi name = mccmsg::DeviceUi::generate();

    auto protocol_id = _db->protocol().getId(d->id().protocol());
    if (protocol_id.isNone())
        return mccmsg::make_error(mccmsg::Error::ProtocolUnknown);

    _queryReg.reset();
    print(binds(&_queryReg, ":name", name.toStringRepr().view(), sqlite3pp::copy));
    print(binds(&_queryReg, ":info", bmcl::StringView(d->id().value()), sqlite3pp::copy));
    print(binds(&_queryReg, ":protocol_id", protocol_id.unwrap()));
    print(binds(&_queryReg, ":binary", d->data().view(), sqlite3pp::nocopy));

    auto r = _queryReg.insert();
    if (r.isErr())
        return mccmsg::make_error(mccmsg::Error::CantRegister, fmt::format("{} {}", _queryReg.sql().non_null(), _queryReg.err_msg().unwrapOr("")));
    id = r.unwrap();
    return name;
}

}
}
