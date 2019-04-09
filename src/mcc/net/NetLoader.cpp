#include <set>
#include <utility>
#include <caf/delegated.hpp>
#include <fmt/format.h>

#include "mcc/plugin/PluginCache.h"
#include "mcc/msg/TmView.h"
#include "mcc/msg/ProtocolController.h"
#include "mcc/msg/ptr/TmSession.h"
#include "mcc/msg/ptr/Protocol.h"
#include "mcc/msg/ptr/Tm.h"
#include "mcc/msg/ptr/NoteVisitor.h"
#include "mcc/msg/ptr/ReqVisitor.h"

#include "mcc/net/Group.h"
#include "mcc/net/NetLoader.h"
#include "mcc/net/NetPlugin.h"
#include "mcc/net/NetLogger.h"
#include "mcc/net/NetLoggerInf.h"
#include "mcc/net/db/DbObj.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::ProtocolDescription);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::DbReqPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::DevReqPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::CancelPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Request_StatePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NotificationPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::tm::LogPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Device);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(bmcl::Rc<mccmsg::ITmView>);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(bmcl::Rc<mccmsg::ITmViewUpdate>);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(bmcl::Rc<const mccmsg::ProtocolController>);

namespace mccnet {

const char* NetLoader::name() const
{
    return "net.loader";
}

void NetLoader::on_exit()
{
    _protocols.clear();
    _ui.clear();
    _devices.clear();
    _channels.clear();
    destroy(_logger);
    destroy(_db);
    destroy(_group);
}

NetLoader::NetLoader(caf::actor_config& cfg, const caf::actor& logger)
    : caf::event_based_actor(cfg)
    , _logger(logger)
{
    _db = spawn<mccdb::DbObj, caf::monitored/* + caf::detached*/>();
    _group = createGroupActor(this, this);
    monitor(_logger);
    monitor(_db);
    monitor(_group);
    assert(_db);
    auto grp = system().groups().get_local("notes");
    join(grp);
}

NetLoader::~NetLoader()
{
}

template<typename... A>
void route_db_and_ui(caf::event_based_actor* self, const std::vector<caf::actor>& ui, A&&... args)
{
    for (auto& i : ui)
        self->send(i, std::forward<A>(args)...);
}

class ReqVisitorNet : public mccmsg::ReqVisitor
{
private:
    const mccmsg::DbReqPtr& _ptr;
    NetLoader* _self;
public:
    ReqVisitorNet(const mccmsg::DbReqPtr& ptr, NetLoader* self)
        : mccmsg::ReqVisitor([ptr, self](const mccmsg::DbReq*) { self->delegate(self->_db, ptr); }), _ptr(ptr), _self(self) {}

    using mccmsg::ReqVisitor::visit;

    void visit(const mccmsg::device::Activate_Request* req) override
    {
        const auto& i = _self->_devices.find(req->data()._name);
        if (i == _self->_devices.end() || !i->second)
            _self->response(caf::sec::request_receiver_down);
        else
            _self->delegate(i->second, _ptr);
    }
    void visit(const mccmsg::channel::Activate_Request* req) override
    {
        const auto& i = _self->_channels.find(req->data()._name);
        if (i == _self->_channels.end() || !i->second)
            _self->response(caf::sec::request_receiver_down);
        else
            _self->delegate(i->second, _ptr);
    }
};

class NoteVisitorNet : public mccmsg::NoteVisitor
{
private:
    NetLoader* _self;
    const mccmsg::NotificationPtr& _ptr;
public:
    explicit NoteVisitorNet(NetLoader* self, const mccmsg::NotificationPtr& ptr)
        : mccmsg::NoteVisitor(
            [this](const mccmsg::Notification*)
            {
                route_db_and_ui(_self, _self->_ui, _ptr);
            })
        , _self(self), _ptr(ptr) {}

    using mccmsg::NoteVisitor::visit;

    void visit(const mccmsg::device::State*) override
    {
        route_db_and_ui(_self, _self->_ui, _ptr);
    }
    void visit(const mccmsg::channel::State*) override
    {
        route_db_and_ui(_self, _self->_ui, _ptr);
    }
    void visit(const mccmsg::tm::Log* note) override
    {
        route_db_and_ui(_self, _self->_ui, _ptr);
        _self->send(_self->_logger, _self->current_sender()->aid, mccmsg::tm::LogPtr(note));
    }
    void visit(const mccmsg::tm::Item*) override
    {
        route_db_and_ui(_self, _self->_ui, _ptr);
    }
};

caf::behavior NetLoader::make_behavior()
{
    set_down_handler([this](caf::down_msg& dm)
    {
        const auto& addr = dm.source;
        std::string reason = system().render(dm.reason);
        {
            auto i = std::find_if(_ui.begin(), _ui.end(), [&](const caf::actor& a) { return a == addr; });
            if (i != _ui.end())
            {
                _ui.erase(i);
                route_db_and_ui(this, _ui, mccmsg::makeNote(new mccmsg::tm::Log(bmcl::LogLevel::Info, std::string(name()), "отключился ui")));
                return;
            }
        }
        {
            auto i = std::find_if(_devices.begin(), _devices.end(), [&](const decltype (_devices)::value_type& t) -> bool {return t.second == addr; });
            if (i != _devices.end())
            {
                BMCL_DEBUG() << fmt::format("Устройство {} удалёно из обменки: {}", i->first.toStdString(), reason);
                _devices.erase(i);
                return;
            }
        }
        {
            auto i = std::find_if(_channels.begin(), _channels.end(), [&](const decltype (_channels)::value_type& t) -> bool {return t.second == addr; });
            if (i != _channels.end())
            {
                BMCL_DEBUG() << fmt::format("Канал {} удалён из обменки: {}", i->first.toStdString(), reason);
                _channels.erase(i);
                return;
            }
        }
        {
            auto i = std::find_if(_protocols.begin(), _protocols.end(), [addr](const Protocols::value_type& ps) { return ps.second == addr; });
            if (i != _protocols.end())
            {
                BMCL_DEBUG() << fmt::format("Протокол {} удалён из обменки: {}", i->first.toStdString(), reason);
                _protocols.erase(i);
            }
        }

        if (_db == dm.source)
        {
            destroy(_db);
            BMCL_DEBUG() << "db down";
        }

        if (_protocols.empty())
        {
            if (_db)   send_exit(_db, _exitReason);
        }

        if (!_db)
        {
            send_exit(_logger, _exitReason);
            destroy(_logger);
            quit();
        }

    });

    set_default_handler(caf::print_and_drop);

    set_error_handler([this](caf::error& e)
    {
        auto r = current_sender();
        int id = r ? r->address().id() : 0;
        std::string s = system().render(e);
        BMCL_DEBUG() << s << id;
    });

    set_exception_handler([](caf::scheduled_actor*, std::exception_ptr&)->caf::error
    {
        return caf::sec::unexpected_message;
    });

    set_exit_handler ( [this](caf::exit_msg& em)
    {
        _exitReason = em.reason;
        send_exit(_group, em.reason);
        if (_protocols.empty())
        {
            quit();
            return;
        }

        for(const auto& i: _protocols)
        {
            if (i.second)
                send_exit(i.second, em.reason);
        }
    });

    return
    {
        [this](const mccmsg::NotificationPtr& note)
        {
            NoteVisitorNet visitor(this, note);
            note->visit(visitor);
        }
      , [this](const mccmsg::DbRequestPtr& req)
        {
            ReqVisitorNet visitor(req, this);
            req->visit(visitor);
            return caf::delegated<mccmsg::ResponsePtr>{};
        }
      , [this](const mccmsg::DevReqPtr& req) -> caf::result<caf::message>
        {
            if (req->group().isSome())
                return delegate(_group, req);

            auto dev = _devices.find(req->device().unwrap());
            if (dev == _devices.end() || !dev->second)
                return caf::sec::request_receiver_down;

            return delegate(dev->second, req);
        }
      , [this](const mccmsg::CancelPtr& cancel)
        {
            const auto& r = cancel->request();
            if (r->kind() == mccmsg::ReqKind::Dev)
            {
                auto req = bmcl::static_pointer_cast<const mccmsg::DevReq>(cancel->request());
                if (req->group().isSome())
                    delegate(_group, cancel);
                else
                {
                    auto dev = _devices.find(req->device().unwrap());
                    if (dev != _devices.end() && dev->second)
                        delegate(dev->second, cancel);
                }
            }
            else if (r->kind() == mccmsg::ReqKind::Db)
            {
                delegate(_db, cancel);
            }
            else
            {
                assert(false);
            }
        }
      , [this](const mccmsg::Request_StatePtr& state)
        {
            route_db_and_ui(this, _ui, state);
        }
      , [this](const caf::atom_constant<caf::atom("ui")>&)
        {
            caf::actor a = caf::actor_cast<caf::actor>(current_sender());
            _ui.emplace_back(a);
            monitor(a);
            auto p = mccmsg::makeNote(new mccmsg::tm::Log(bmcl::LogLevel::Info, std::string(name()), fmt::format("подключился ui {}", a->id())));
            route_db_and_ui(this, _ui, p);
        }
      , [this](const caf::atom_constant<caf::atom("device")>, const mccmsg::Device& d)
        {
            caf::actor a = caf::actor_cast<caf::actor>(current_sender());
            _devices[d] = a;
            monitor(a);
            auto p = mccmsg::makeNote(new mccmsg::tm::Log(bmcl::LogLevel::Info, std::string(name()), fmt::format("подключился device {}", a->id())));
            route_db_and_ui(this, _ui, p);
        }
      , [this](const caf::atom_constant<caf::atom("channel")>, const mccmsg::Channel& c)
        {
            caf::actor a = caf::actor_cast<caf::actor>(current_sender());
            _channels[c] = a;
            monitor(a);
            auto p = mccmsg::makeNote(new mccmsg::tm::Log(bmcl::LogLevel::Info, std::string(name()), fmt::format("подключился канал обмена {}", a->id())));
            route_db_and_ui(this, _ui, p);
        }
      , [this](const caf::atom_constant<caf::atom("pluginload")>, const bmcl::Rc<const mccplugin::PluginCache>& cache, const bmcl::Rc<const mccmsg::ProtocolController>& controller)
        {
            getPlugins(cache);
            send(_db, caf::atom("pluginload"), controller);
        }
      , [this](log_set_atom, const std::string& folder)
        {
            send(_logger, log_set_atom::value, folder);
        }
    };
}

void NetLoader::getPlugins(const bmcl::Rc<const mccplugin::PluginCache>& cache)
{
    const auto& plugins = cache->plugins();
    for (const auto& plugin : plugins)
    {
        if (!plugin->hasTypeId(mccnet::NetPlugin::id))
            continue;
        const mccnet::NetPlugin* p = static_cast<mccnet::NetPlugin*>(plugin.get());
        _protocols.emplace(p->protocol(), p->getProtocolCreator()(this, this, _logger, _group));
    }
}

}
