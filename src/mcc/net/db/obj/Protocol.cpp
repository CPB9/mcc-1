#include <caf/make_message.hpp>
#include <caf/result.hpp>
#include <caf/event_based_actor.hpp>
#include <bmcl/MakeRc.h>
#include "mcc/msg/ProtocolController.h"
#include "mcc/msg/ptr/Protocol.h"
#include "mcc/net/db/Sql.h"
#include "mcc/net/db/obj/Protocol.h"
#include "mcc/net/db/obj/Device.h"
#include "mcc/plugin/PluginCache.h"
#include "mcc/net/NetPlugin.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::protocol::List_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::protocol::UnRegister_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::protocol::DescriptionList_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::protocol::Description_ResponsePtr);

namespace mccdb {
namespace dbobj {


Protocol::Protocol(DbObjInternal* db)
    : QueryObject(db, "protocol", "shareable, logging, param_info, timeout, pixmap"
                , std::bind(&Protocol::get, this, std::placeholders::_1)
                , std::bind(&Protocol::insert, this, std::placeholders::_1, std::placeholders::_2))
{
}

caf::result<mccmsg::protocol::Register_ResponsePtr> Protocol::execute(const mccmsg::protocol::Register_Request& request)
{
    ObjectId id;
    auto r = insert(request.data(), id);
    if (r.isErr())
        return r.takeErr();
    auto d = getOne(id);
    if (d.isErr())
        return d.takeErr();
    registered(r.unwrap(), true);
    return mccmsg::make<mccmsg::protocol::Register_Response>(&request, d.take());
}

caf::result<mccmsg::protocol::Update_ResponsePtr> Protocol::execute(const mccmsg::protocol::Update_Request&)
{
    return mccmsg::make_error(mccmsg::Error::NotImplemented);
}

caf::result<mccmsg::protocol::UnRegister_ResponsePtr> Protocol::execute(const mccmsg::protocol::UnRegister_Request&)
{
    return mccmsg::make_error(mccmsg::Error::NotImplemented);
}

caf::result<mccmsg::protocol::List_ResponsePtr> Protocol::execute(const mccmsg::protocol::List_Request& request)
{
    auto r = getList();
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::protocol::List_Response>(&request, r.take());
}
caf::result<mccmsg::protocol::Description_ResponsePtr> Protocol::execute(const mccmsg::protocol::Description_Request& request)
{
    auto r = getOne(request.data());
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::protocol::Description_Response>(&request, r.take());
}

caf::result<mccmsg::protocol::DescriptionS_ResponsePtr> Protocol::execute(const mccmsg::protocol::DescriptionS_Request& request)
{
    return mccmsg::Error::NotImplemented;
}

caf::result<mccmsg::protocol::DescriptionList_ResponsePtr> Protocol::execute(const mccmsg::protocol::DescriptionList_Request& request)
{
    auto r = getAll();
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::protocol::DescriptionList_Response>(&request, r.take());
}

bmcl::Option<mccmsg::ProtocolDescription> Protocol::get(const sqlite3pp::selecter::row& r)
{
    bmcl::Uuid n = bmcl::Uuid::createFromString(r.get<bmcl::StringView>("name")).takeOption().unwrapOr(bmcl::Uuid::createNil());
    mccmsg::Protocol name(n);
    mccmsg::PropertyDescriptionPtrs req;
    mccmsg::PropertyDescriptionPtrs opt;

    const auto i = std::find_if(_protocols.begin(), _protocols.end(), [&name](const mccmsg::ProtocolDescription& p) { return p->name() == name; });
    if (i != _protocols.end())
    {
        req = (*i)->requiredProperties();
        opt = (*i)->optionalProperties();
    }

     auto d = bmcl::makeRc<const mccmsg::ProtocolDescriptionObj>(name
                                    , r.get<bool>("shareable")
                                    , r.get<bool>("logging")
                                    , std::chrono::milliseconds(r.get<int64_t>("timeout"))
                                    , r.get<bmcl::StringView>("info")
                                    , r.get<bmcl::StringView>("param_info")
                                    , r.get<bmcl::Bytes>("pixmap")
                                    , std::move(req)
                                    , std::move(opt)
    );
    return d;
}

bmcl::Result<mccmsg::Protocol, caf::error> Protocol::insert(const mccmsg::ProtocolDescription& d, ObjectId& id)
{
    _queryReg.reset();
    print(binds(&_queryReg, ":name", d->name().toStringRepr().view(), sqlite3pp::copy));
    print(binds(&_queryReg, ":shareable", d->shareable()));
    print(binds(&_queryReg, ":logging", d->logging()));
    print(binds(&_queryReg, ":info", bmcl::StringView(d->info()), sqlite3pp::nocopy));
    print(binds(&_queryReg, ":param_info", bmcl::StringView(d->param_info()), sqlite3pp::nocopy));
    print(binds(&_queryReg, ":timeout", d->timeout().count()));
    print(binds(&_queryReg, ":pixmap", d->pixmap().asBytes(), sqlite3pp::nocopy));

    auto r = _queryReg.insert();
    if (r.isErr())
        return mccmsg::make_error(mccmsg::Error::CantRegister, fmt::format("{} {}", _queryReg.sql().non_null(), _queryReg.err_msg().unwrapOr("")));
    id = r.take();
    return d->name();
}

void Protocol::updatePlugins(const mccmsg::ProtocolController* pc)
{
    for (const auto& i : pc->dscrs())
    {
        _protocols.push_back(i);

        auto tmp = getId(i->name());
        if (tmp.isSome())
        {
            BMCL_WARNING() << fmt::format("Протокол {} уже зарегистрирован", i->name().toStdString());
            continue;
        }

        ObjectId id;
        auto r = insert(i, id);
        if (r.isErr())
        {
            BMCL_WARNING() << fmt::format("Не удалось зарегистрировать протокол {}: {}", i->name().toStdString(), _db->self()->system().render(r.takeErr()));
            continue;;
        }
        registered(r.unwrap(), true);
    }
}

}
}
