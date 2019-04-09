#include <caf/make_message.hpp>
#include <caf/result.hpp>
#include <bmcl/MakeRc.h>
#include "mcc/msg/ptr/Radar.h"
#include "mcc/net/db/Sql.h"
#include "mcc/net/db/obj/Radar.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::radar::List_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::radar::UnRegister_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::radar::DescriptionList_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::radar::Description_ResponsePtr);

namespace mccdb {
namespace dbobj {

Radar::Radar(DbObjInternal* db)
    : QueryObject(db, "radar", "settings", Radar::get, std::bind(&Radar::insert, this, std::placeholders::_1, std::placeholders::_2))
    , _unregister(db->db())
    , _update(db->db())
{
    sql_prepare(_unregister, "delete from radar where name=:name;");
    sql_prepare(_update, "update radar set settings=:settings, info=:info where name=:name");
}

caf::result<mccmsg::radar::Register_ResponsePtr> Radar::execute(const mccmsg::radar::Register_Request& request)
{
    ObjectId id;
    auto r = insert(request.data(), id);
    if (r.isErr())
        return r.takeErr();
    auto d = getOne(id);
    if (d.isErr())
        return d.takeErr();
    registered(r.unwrap(), true);
    return mccmsg::make<mccmsg::radar::Register_Response>(&request, d.take());
}

caf::result<mccmsg::radar::UnRegister_ResponsePtr> Radar::execute(const mccmsg::radar::UnRegister_Request&)
{
    return mccmsg::make_error(mccmsg::Error::NotImplemented);
}

caf::result<mccmsg::radar::Update_ResponsePtr> Radar::execute(const mccmsg::radar::Update_Request& request)
{
    const mccmsg::RadarDescription& dscr = request.data().dscr();
    _update.reset();
    print(binds(&_update, ":name", dscr->name().toStringRepr().view(), sqlite3pp::copy));
    print(binds(&_update, ":info", bmcl::StringView(dscr->info()), sqlite3pp::nocopy));
    print(binds(&_update, ":settings", bmcl::StringView(dscr->settings()), sqlite3pp::nocopy));
    auto r = print(exec(&_update));
    if (r.isSome())
        return mccmsg::make_error(mccmsg::Error::CantRegister, r.take());

    auto v = updated(dscr->name());
    if (v.isNone())
        return mccmsg::make_error(mccmsg::Error::CantUpdate);

    return mccmsg::make<mccmsg::radar::Update_Response>(&request, v.take());
}

caf::result<mccmsg::radar::List_ResponsePtr> Radar::execute(const mccmsg::radar::List_Request& request)
{
    auto r = getList();
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::radar::List_Response>(&request, r.take());
}

caf::result<mccmsg::radar::Description_ResponsePtr> Radar::execute(const mccmsg::radar::Description_Request& request)
{
    auto r = getOne(request.data());
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::radar::Description_Response>(&request, r.take());
}

caf::result<mccmsg::radar::DescriptionS_ResponsePtr> Radar::execute(const mccmsg::radar::DescriptionS_Request& request)
{
    return mccmsg::Error::NotImplemented;
}

caf::result<mccmsg::radar::DescriptionList_ResponsePtr> Radar::execute(const mccmsg::radar::DescriptionList_Request& request)
{
    auto r = getAll();
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::radar::DescriptionList_Response>(&request, r.take());
}

bmcl::Option<mccmsg::RadarDescription> Radar::get(const sqlite3pp::selecter::row& r)
{
    bmcl::Uuid name = bmcl::Uuid::createFromString(r.get<bmcl::StringView>("name")).takeOption().unwrapOr(bmcl::Uuid::createNil());
    return bmcl::makeRc<const mccmsg::RadarDescriptionObj>(mccmsg::Radar(name), r.get<bmcl::StringView>("info"), r.get<bmcl::StringView>("settings"));
}

bmcl::Result<mccmsg::Radar, caf::error> Radar::insert(const mccmsg::RadarDescription& d, ObjectId& id)
{
    mccmsg::Radar name = mccmsg::Radar::generate();

    _queryReg.reset();
    print(binds(&_queryReg, ":name", name.toStringRepr().view(), sqlite3pp::copy));
    print(binds(&_queryReg, ":info", bmcl::StringView(d->info()), sqlite3pp::nocopy));
    print(binds(&_queryReg, ":settings", bmcl::StringView(d->settings()), sqlite3pp::nocopy));

    auto r = _queryReg.insert();
    if (r.isErr())
        return mccmsg::make_error(mccmsg::Error::CantRegister, fmt::format("{} {}", _queryReg.sql().non_null(), _queryReg.err_msg().unwrapOr("")));
    id = r.take();
    return name;
}

}
}
