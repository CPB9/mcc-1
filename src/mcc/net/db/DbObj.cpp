#include <chrono>
#include <QResource>
#include <sqlite3pp.h>

#include <bmcl/Utils.h>
#include "mcc/msg/ptr/Channel.h"
#include "mcc/msg/ptr/Device.h"
#include "mcc/msg/ptr/DeviceUi.h"
#include "mcc/msg/ptr/Firmware.h"
#include "mcc/msg/ptr/Protocol.h"
#include "mcc/msg/ptr/Radar.h"
#include "mcc/msg/ptr/TmSession.h"
#include "mcc/msg/ptr/ReqVisitor.h"
#include "mcc/msg/ptr/NoteVisitor.h"

#include "mcc/path/Paths.h"

#include "mcc/net/db/DbObj.h"
#include "mcc/net/db/obj/DbObjInternal.h"
#include "mcc/net/db/obj/Device.h"
#include "mcc/net/db/obj/DeviceUi.h"
#include "mcc/net/db/obj/Firmware.h"
#include "mcc/net/db/obj/Protocol.h"
#include "mcc/net/db/obj/Channel.h"
#include "mcc/net/db/obj/Radar.h"
#include "mcc/net/db/obj/TmSession.h"
#include "mcc/net/db/obj/Advanced.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NotificationPtr)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Device)

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::tmSession::Register_RequestPtr)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::tmSession::UnRegister_RequestPtr)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::tmSession::List_RequestPtr)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::tmSession::Description_RequestPtr)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::tmSession::DescriptionList_RequestPtr)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::tmSession::Update_RequestPtr)

MCC_INIT_QRESOURCES(Db);

namespace mccdb {

using namespace dbobj;

const char* DbObj::name() const
{
    return "net.db.obj";
}

DbObj::DbObj(caf::actor_config& cfg, std::string&& binPath)
        : caf::event_based_actor(cfg), _binPath(std::move(binPath))
{
    auto f = [](sqlite3pp::Error err, const char* msg)
    {
        BMCL_WARNING() << fmt::format("{}({}) {}", (int)err, sqlite3pp::to_string(err), msg).c_str();
    };
    sqlite3pp::database::set_error_handler(f);

    _db = std::make_unique<dbobj::DbObjInternal>(this);
    if (_db->open(_binPath + "/" + _dbName, _dbSchema))
        return;
    assert(false);
}

DbObj::DbObj(caf::actor_config& cfg)
    : DbObj(cfg, mccpath::getConfigPath())
{
}

DbObj::~DbObj()
{
}

void DbObj::on_exit()
{
    _db->close();
    _db.release();
}

class ReqVisitorDb : public mccmsg::ReqVisitor
{
private:
    DbObj* _self;
public:
    ReqVisitorDb(DbObj* self)
        : mccmsg::ReqVisitor([self](const mccmsg::DbReq*) { self->response(caf::sec::unexpected_message); }), _self(self) {}

#define EXEC_RET if (r.err) _self->response(r.err); else _self->response(r.value);
#define EXEC_STANDART_SET(nmsp) \
    void visit(const mccmsg::nmsp::Register_Request* msg) override { auto r = _self->_db->nmsp().execute(*msg); EXEC_RET; }        \
    void visit(const mccmsg::nmsp::UnRegister_Request* msg) override { auto r = _self->_db->nmsp().execute(*msg); EXEC_RET; }      \
    void visit(const mccmsg::nmsp::List_Request* msg) override { auto r = _self->_db->nmsp().execute(*msg); EXEC_RET; }            \
    void visit(const mccmsg::nmsp::Description_Request* msg) override { auto r = _self->_db->nmsp().execute(*msg); EXEC_RET; }     \
    void visit(const mccmsg::nmsp::DescriptionS_Request* msg) override { auto r = _self->_db->nmsp().execute(*msg); EXEC_RET; }    \
    void visit(const mccmsg::nmsp::DescriptionList_Request* msg) override { auto r = _self->_db->nmsp().execute(*msg); EXEC_RET; } \
    void visit(const mccmsg::nmsp::Update_Request* msg) override { auto r = _self->_db->nmsp().execute(*msg); EXEC_RET; }

    void visit(const mccmsg::device::Connect_Request* msg) override { auto r = _self->_db->device().execute(*msg); EXEC_RET; }
    void visit(const mccmsg::advanced::ChannelAndDeviceRegister_Request* msg) override { auto r = _self->_db->advanced().execute(*msg); EXEC_RET;}

    EXEC_STANDART_SET(protocol)
    EXEC_STANDART_SET(channel)
    EXEC_STANDART_SET(radar)
    EXEC_STANDART_SET(tmSession)
    EXEC_STANDART_SET(firmware)
    EXEC_STANDART_SET(device)
    EXEC_STANDART_SET(deviceUi)

#undef EXEC_STANDART_SET
#undef EXEC_REQ
};

caf::behavior DbObj::make_behavior()
{
    set_error_handler( [this](caf::error& e)
    {
        auto r = current_sender();
        int id = r->address().id();
        std::string s = system().render(e);
        BMCL_DEBUG() << s << id;
    });

    set_down_handler([this](caf::down_msg&) {});

    set_default_handler(caf::print_and_drop);

    return{
            [this](const mccmsg::DbReqPtr& req)
            {
                ReqVisitorDb visitor(this);
                req->visit(visitor);
                return caf::delegated<void>();
            }
          , [this](const mccmsg::NotificationPtr&)
            {
            }
          , [this](const caf::atom_constant<caf::atom("pluginload")>, const bmcl::Rc<const mccmsg::ProtocolController>& controller)
            {
                _db->protocol().updatePlugins(controller.get());
                _db->firmware().updatePlugins(controller.get());
            }
    };
}

}
