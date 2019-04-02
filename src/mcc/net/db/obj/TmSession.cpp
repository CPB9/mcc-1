#include <QDir>
#include <caf/make_message.hpp>
#include <caf/event_based_actor.hpp>
#include <caf/result.hpp>
#include <bmcl/MakeRc.h>
#include "mcc/msg/ptr/TmSession.h"
#include "mcc/path/Paths.h"
#include "mcc/net/db/DbTime.h"
#include "mcc/net/db/Sql.h"
#include "mcc/net/db/obj/TmSession.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::TmSession);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::tmSession::List_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::tmSession::UnRegister_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::tmSession::DescriptionList_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::tmSession::DescriptionList_RequestPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::tmSession::Description_ResponsePtr);

namespace mccdb {
namespace dbobj {

bmcl::Result<mccmsg::TmSessionDescription, std::string> getDir(const QDir& parent, const QFileInfo& dir)
{
    auto qsessionName = dir.fileName();
    auto sessionName = qsessionName.toStdString();
    auto session = mccmsg::TmSessionDescriptionObj::getTmSession(sessionName);
    if (session.isNone())
    {
        return "Некорректное имя каталога сессии: " + sessionName;
    }

    QDir sessionDir = parent;
    if (!sessionDir.cd(qsessionName))
        return "Не удалось открыть каталог: " + sessionName;

    std::string screen;
    mccmsg::Devices devices;
    mccmsg::Channels channels;

    QFileInfoList files = sessionDir.entryInfoList();
    for(const auto& i: files)
    {
        std::string fileName = i.fileName().toStdString();
        if (fileName == "about.txt")
        {
            //извлечь описание каталога
            continue;
        }
        if (fileName == "screen.avi")
        {   //извлечь описание записи экрана
            screen = i.fileName().toStdString();
            continue;
        }

        auto d = mccmsg::TmSessionDescriptionObj::getDevice(fileName);
        if (d.isSome())
        {
            devices.push_back(d.unwrap());
            continue;
        }

        auto c = mccmsg::TmSessionDescriptionObj::getChannel(fileName);
        if (c.isSome())
        {
            channels.push_back(c.unwrap());
            continue;
        }
    }

    return bmcl::makeRc<const mccmsg::TmSessionDescriptionObj>(mccmsg::TmSession(session.unwrap())
                                       , bmcl::StringView()
                                       , dir.fileName().toStdString()
                                       , !screen.empty()
                                       , devices
                                       , channels
                                       , bmcl::SystemClock::now(), bmcl::None);
}

mccmsg::TmSessionDescriptions TmSession::getDirs()
{
    QDir dataDir(mccpath::qGetLogsPath());
    dataDir.setFilter(QDir::Dirs | QDir::CaseSensitive);
    QFileInfoList dirs = dataDir.entryInfoList();

    mccmsg::TmSessionDescriptions sessions;
    for(const QFileInfo& i: dirs)
    {
        auto r = getDir(dataDir, i);
        if (r.isErr())
        {
            BMCL_WARNING() << r.unwrapErr();
            continue;
        }
        sessions.emplace_back(r.take());
    }
    return sessions;
}

bmcl::Option<mccmsg::Error> TmSession::removeDir(const mccmsg::TmSession& session)
{
    QDir dataDir(mccpath::qGetLogsPath());
    dataDir.setFilter(QDir::Dirs | QDir::CaseSensitive);
    QFileInfoList dirs = dataDir.entryInfoList();
    for(const auto& i: dirs)
    {
        QString fileName = i.fileName();
        auto uuid = mccmsg::TmSessionDescriptionObj::getTmSession(fileName.toStdString());
        if (uuid.isNone())
            continue;

        bool r = dataDir.cd(fileName);
        assert(r);
        dataDir.removeRecursively();
        return bmcl::None;
    }
    return mccmsg::Error::TmSessionUnknown;
}

TmSession::TmSession(DbObjInternal* db)
    : QueryObject(db, "tm_session", "folder, started, finished", TmSession::get, std::bind(&TmSession::insert, this, std::placeholders::_1, std::placeholders::_2))
    , _unregister(db->db())
    , _update(db->db())
    , _closeSessions(db->db())
{
    sql_prepare(_unregister, "delete from tm_session where name=:name;");
    sql_prepare(_update, "update tm_session set info=:info where name=:name");
    sql_prepare(_closeSessions, "update tm_session set finished=:finished where finished is NULL");

    sync(getDirs());
}

void TmSession::closeAllSessions()
{
    _closeSessions.reset();
    print(binds(&_closeSessions, ":finished", serializeTime(bmcl::SystemClock::now()), sqlite3pp::copy));
    auto r = print(exec(&_closeSessions));
    if (r.isSome())
    {
        BMCL_WARNING() << "не удалось закрыть незакрытые сессии";
    }
}

void TmSession::sync(const mccmsg::TmSessionDescriptions& folders)
{
    for (const auto& i : folders)
    {
        auto id = getId(i->name());
        if (id.isSome())
            continue;

        ObjectId newid;
        insert(i, newid);
        registered(i->name(), true);
    }
    closeAllSessions();

    const auto r = getAll();
    if (r.isErr())
        return;

    for (const auto& i : r.unwrap())
    {
        const auto j = std::find_if(folders.begin(), folders.end(), [&i](const mccmsg::TmSessionDescription& f) { return f->folder() == i->folder(); });
        if (j != folders.end())
            continue;
        removeOne(i->name());
        registered(i->name(), false);
    }
}

caf::result<mccmsg::tmSession::Register_ResponsePtr> TmSession::execute(const mccmsg::tmSession::Register_Request& request)
{
    closeAllSessions();

    mccmsg::TmSession session = mccmsg::TmSession::generate();
    std::string folder = mccmsg::TmSessionDescriptionObj::genTmSessionFile(session);

    QDir dataDir(mccpath::qGetLogsPath());
    dataDir.setFilter(QDir::Dirs | QDir::CaseSensitive);

    bool isCreated = dataDir.mkdir(QString::fromStdString(folder));
    if (!isCreated)
        return mccmsg::make_error(mccmsg::Error::CantRegister);

    auto dscr = bmcl::makeRc<mccmsg::TmSessionDescriptionObj>(session, request.data()->info(), folder, bmcl::SystemClock::now());

    ObjectId id;
    auto r = insert(dscr, id);
    if (r.isErr())
    {
        auto err = removeDir(r.unwrap());
        if (err.isSome())
            return mccmsg::make_error(err.take());
        return mccmsg::make_error(mccmsg::Error::CantRegister);
    }
    registered(session, true);
    _db->setTmSession(folder);
    return mccmsg::make<mccmsg::tmSession::Register_Response>(&request, dscr);
}

caf::result<mccmsg::tmSession::UnRegister_ResponsePtr> TmSession::execute(const mccmsg::tmSession::UnRegister_Request& request)
{
    {
        auto r = removeDir(request.data());
        if (r.isSome())
            return mccmsg::make_error(r.unwrap());
    }

    auto r = removeOne(request.data());
    if (r.isSome())
        return r.unwrap();
    registered(request.data(), false);
    return mccmsg::make<mccmsg::tmSession::UnRegister_Response>(&request);
}

caf::result<mccmsg::tmSession::Update_ResponsePtr> TmSession::execute(const mccmsg::tmSession::Update_Request& request)
{
    const mccmsg::TmSessionDescription& dscr = request.data().dscr();
    _update.reset();
    print(binds(&_update, ":name", dscr->name().toStdString(), sqlite3pp::copy));
    print(binds(&_update, ":info", dscr->info().c_str(), sqlite3pp::nocopy));
    auto r = print(exec(&_update));
    if (r.isSome())
        return mccmsg::make_error(mccmsg::Error::CantUpdate, r.take());

    auto v = updated(dscr->name());
    if (v.isNone())
        return mccmsg::make_error(mccmsg::Error::CantUpdate);

    return mccmsg::make<mccmsg::tmSession::Update_Response>(&request, v.take());
}

caf::result<mccmsg::tmSession::List_ResponsePtr> TmSession::execute(const mccmsg::tmSession::List_Request& request)
{
    auto r = getList();
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::tmSession::List_Response>(&request, r.take());
}

caf::result<mccmsg::tmSession::Description_ResponsePtr> TmSession::execute(const mccmsg::tmSession::Description_Request& request)
{
    auto r = getOne(request.data());
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::tmSession::Description_Response>(&request, r.take());
}

caf::result<mccmsg::tmSession::DescriptionS_ResponsePtr> TmSession::execute(const mccmsg::tmSession::DescriptionS_Request& request)
{
    return mccmsg::Error::NotImplemented;
}

caf::result<mccmsg::tmSession::DescriptionList_ResponsePtr> TmSession::execute(const mccmsg::tmSession::DescriptionList_Request& request)
{
    auto r = getAll();
    if (r.isErr()) return r.takeErr();
    return mccmsg::make<mccmsg::tmSession::DescriptionList_Response>(&request, r.take());
}

bmcl::Option<mccmsg::TmSessionDescription> TmSession::get(const sqlite3pp::selecter::row& r)
{
    bmcl::Uuid name = bmcl::Uuid::createFromString(r.get<bmcl::StringView>("name")).takeOption().unwrapOr(bmcl::Uuid::createNil());
    bmcl::SystemTime started = deserializeTime(r.get<bmcl::StringView>("started"));
    bmcl::Option<bmcl::SystemTime> finished;
    if (!r.is_null("finished"))
        finished = deserializeTime(r.get<bmcl::StringView>("finished"));
    return bmcl::makeRc<const mccmsg::TmSessionDescriptionObj>(mccmsg::TmSession(name), r.get<bmcl::StringView>("info"), r.get<bmcl::StringView>("folder"), started, finished);
}

bmcl::Result<mccmsg::TmSession, caf::error> TmSession::insert(const mccmsg::TmSessionDescription& d, ObjectId& id)
{
    _queryReg.reset();
    print(binds(&_queryReg, ":name", d->name().toStdString(), sqlite3pp::copy));
    print(binds(&_queryReg, ":info", d->info(), sqlite3pp::nocopy));
    print(binds(&_queryReg, ":folder", d->folder(), sqlite3pp::nocopy));
    print(binds(&_queryReg, ":started", serializeTime(d->started()), sqlite3pp::copy));
    if (d->finished().isSome())
        print(binds(&_queryReg, ":finished", serializeTime(d->finished().unwrap()), sqlite3pp::copy));

    auto r = _queryReg.insert();
    if (r.isErr())
        return mccmsg::make_error(mccmsg::Error::CantRegister, fmt::format("{} {}", _queryReg.sql().non_null(), _queryReg.err_msg().unwrapOr("")));
    id = r.take();
    return d->name();
}

}
}
