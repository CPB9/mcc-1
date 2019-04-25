#pragma once
#include "mcc/msg/ptr/Fwd.h"
#include "mcc/msg/obj/TmSession.h"
#include "mcc/net/db/Sql.h"
#include "mcc/net/db/DbHandler.h"
#include "mcc/net/db/obj/Object.h"

namespace mccdb {
namespace dbobj {

class TmSession : public QueryObject<mccmsg::TmSessionDescription, mccmsg::TmSession>
{
public:
    TmSession(DbObjInternal*);
    caf::result<mccmsg::tmSession::Register_ResponsePtr> execute(const mccmsg::tmSession::Register_Request& request);
    caf::result<mccmsg::tmSession::UnRegister_ResponsePtr> execute(const mccmsg::tmSession::UnRegister_Request& request);
    caf::result<mccmsg::tmSession::Update_ResponsePtr> execute(const mccmsg::tmSession::Update_Request& request);
    caf::result<mccmsg::tmSession::List_ResponsePtr> execute(const mccmsg::tmSession::List_Request& request);
    caf::result<mccmsg::tmSession::Description_ResponsePtr> execute(const mccmsg::tmSession::Description_Request& request);
    caf::result<mccmsg::tmSession::DescriptionS_ResponsePtr> execute(const mccmsg::tmSession::DescriptionS_Request& request);
    caf::result<mccmsg::tmSession::DescriptionList_ResponsePtr> execute(const mccmsg::tmSession::DescriptionList_Request& request);
    bmcl::Result<mccmsg::TmSession, caf::error> insert(const mccmsg::TmSessionDescription& r, ObjectId& id);
private:
    bmcl::Result<bool, std::string> updateInfo(ObjectId device_id, const mccmsg::TmSessionDescription& old, const std::string& info);
    bmcl::Result<bool, std::string> updateFinalTime(ObjectId device_id, const mccmsg::TmSessionDescription& old, const bmcl::Option<bmcl::SystemTime>& finished);

    mccmsg::TmSessionDescriptions getDirs();
    bmcl::Option<mccmsg::Error> removeDir(const mccmsg::TmSession& session);
    void closeAllSessions();
    void sync(const mccmsg::TmSessionDescriptions&);
    static bmcl::Option<mccmsg::TmSessionDescription> get(const sqlite3pp::selecter::row& r);

    sqlite3pp::statement _unregister;
    sqlite3pp::statement _updateInfo;
    sqlite3pp::statement _updateFinish;
    sqlite3pp::statement _closeSessions;
};

}
}
