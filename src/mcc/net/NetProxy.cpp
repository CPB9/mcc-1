#include <caf/actor.hpp>
#include <caf/actor_system.hpp>
#include <caf/actor_system_config.hpp>
#include <caf/actor_ostream.hpp>
#include <caf/send.hpp>
#include <caf/io/middleman.hpp>
#include <QtGlobal>
#include <bmcl/MakeRc.h>

#include "mcc/net/Error.h"
#include "mcc/net/NetProxy.h"
#include "mcc/net/NetLoader.h"
#include "mcc/net/NetLogger.h"
#include "mcc/msg/ProtocolController.h"

#include "mcc/plugin/PluginCache.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(bmcl::Rc<const mccmsg::ProtocolController>);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(bmcl::Rc<const mccplugin::PluginCache>);

namespace mccnet {

struct PCI
{
    FrmCreator f;
    ProtCreator p;
    TmStorCreator t;
    mccmsg::ProtocolDescription d;
};
using PCIs = std::map<mccmsg::Protocol, PCI>;

class ProtocolControllerImpl : public mccmsg::ProtocolController
{
public:
    ProtocolControllerImpl(PCIs&& ps, mccmsg::ProtocolDescriptions&& dscrs) : mccmsg::ProtocolController(std::move(dscrs)), _ps(std::move(ps))
    {
        for (const auto& i : _ps)
        {
            for (const auto& j : i.second.d->requiredProperties())
            {
                _properties[j->name()] = j;
            }
            
        }
    }
    bmcl::OptionRc<const mccmsg::IPropertyValue> decodeProperty(const mccmsg::Property& p, bmcl::StringView s) const override
    {
        const auto i = _properties.find(p);
        if (i == _properties.end())
            return bmcl::None;
        return i->second->decode(s);
    }
    bmcl::OptionRc<const mccmsg::IFirmware> decodeFirmware(const mccmsg::ProtocolValue& id, bmcl::Bytes bytes) const override
    {
        const auto i = _ps.find(id.protocol());
        if (i == _ps.end())
            return bmcl::None;
        return i->second.f(id, bytes);
    }
    bmcl::OptionRc<mccmsg::ITmStorage> createStorage(const mccmsg::Protocol& p, const mccmsg::ITmView* view) const override
    {
        const auto i = _ps.find(p);
        if (i == _ps.end())
            return bmcl::None;
        return i->second.t(bmcl::wrapRc<const mccmsg::ITmView>(view));
    }

private:
    PCIs _ps;
    std::map<mccmsg::Property, mccmsg::PropertyDescriptionPtr> _properties;
};

bmcl::Rc<mccmsg::ProtocolController> createController(mccplugin::PluginCache* cache)
{
    mccmsg::ProtocolDescriptions dscrs;
    PCIs ps;

    const auto& plugins = cache->plugins();
    for (const auto& plugin : plugins)
    {
        if (!plugin->hasTypeId(mccnet::NetPlugin::id))
            continue;
        const mccnet::NetPlugin* p = static_cast<mccnet::NetPlugin*>(plugin.get());
        dscrs.emplace_back(p->description());
        PCI pc;
        pc.f = p->getFirmwareCreator();
        pc.d = p->description();
        pc.t = p->getTmStorageCreator();
        pc.p = p->getProtocolCreator();
        ps.emplace(p->protocol(), std::move(pc));
    }

    return bmcl::makeRc<ProtocolControllerImpl>(std::move(ps), std::move(dscrs));
}

NetProxy::NetProxy(bool isConsole, bmcl::Option<uint16_t> maxThreads, mccplugin::PluginCache* cache, bmcl::Option<uint16_t> port) : _qtLog(false), _bmclLog(false), _internal(true)
{
    _cfg = std::make_unique<caf::actor_system_config>();
    _cfg->set("logger.file-verbosity", caf::atom("quiet"));
    _cfg->set("logger.console-verbosity", caf::atom("quiet"));
    _cfg->set("logger.console", caf::atom("colored"));
    _cfg->load<caf::io::middleman>();

    if (maxThreads.isSome())
        _cfg->set("scheduler.max-threads", maxThreads.unwrap());

    _sys = std::make_unique<caf::actor_system>(*_cfg);
    mccmsg::add_renderer(*_cfg);
    caf::actor_ostream::redirect_all(*_sys, ":log");
    _logger = _sys->spawn <mccnet::Logger, caf::detached>(isConsole);
    _loader = _sys->spawn<mccnet::NetLoader>(_logger);

    setBmclLogHandler();
    setQtLogHandler();
    loadPlugins(cache);

    if (port.isSome())
    {
        auto r = _sys->middleman().publish(_loader, port.unwrap());
        if (!r.engaged())
        {
            assert(false);
            BMCL_DEBUG() << "unable to bind port " << port.unwrap();
            return;
        }
    }
}

NetProxy::NetProxy(bool isConsole, bmcl::Option<uint16_t> maxThreads, mccplugin::PluginCache* cache, const std::string& host, uint16_t port) : _qtLog(false), _bmclLog(false), _internal(false)
{
    _cfg = std::make_unique<caf::actor_system_config>();
    _cfg->set("logger.file-verbosity", caf::atom("quiet"));
    _cfg->set("logger.console-verbosity", caf::atom("quiet"));
    _cfg->set("logger.console", caf::atom("colored"));
    _cfg->load<caf::io::middleman>();

    if (maxThreads.isSome())
        _cfg->set("scheduler.max-threads", maxThreads.unwrap());

    _sys = std::make_unique<caf::actor_system>(*_cfg);
    mccmsg::add_renderer(*_cfg);
//     caf::actor_ostream::redirect_all(*_sys, ":log");

//     setBmclLogHandler();
//     setQtLogHandler();
    loadPlugins(cache);

    auto r = _sys->middleman().remote_actor(host, port);
    if (!r.engaged())
    {
        BMCL_DEBUG() << "unable to connect to remote core: " << _sys->render(r.cerror());
        return;
    }

    const caf::actor& core = r.cvalue();
    if (!core)
    {
        assert(false);
        BMCL_DEBUG() << "unable to connect to remote core";
        return;
    }

    _loader = core;
    _logger = core;
}

NetProxy::~NetProxy()
{
    if (_internal && _loader) caf::anon_send_exit(_loader, caf::exit_reason::user_shutdown);
    //if (_ui) caf::anon_send_exit(*_ui, caf::exit_reason::user_shutdown);
    destroy(_loader);
    destroy(_logger);
    if (_sys) _sys->await_all_actors_done();

    if (_qtLog) qInstallMessageHandler(0);
    if (_bmclLog) bmcl::setDefaulLogHandler();
    std::cout.flush();
    std::cerr.flush();

    _sys.reset();
    _cfg.reset();
}

void NetProxy::loadPlugins(mccplugin::PluginCache* cache)
{
    auto r = createController(cache);
    cache->addPluginData(std::make_unique<mccmsg::ProtocolControllerPluginData>(r.get()));
    if (_internal)
        caf::anon_send(_loader, caf::atom("pluginload"), bmcl::wrapRc<const mccplugin::PluginCache>(cache), bmcl::wrapRc<const mccmsg::ProtocolController>(r.get()));
}

const caf::actor& NetProxy::core() const
{
    return _loader;
}

const caf::actor& NetProxy::logger() const
{
    return _logger;
}

const caf::actor_system& NetProxy::actor_system() const
{
    return *_sys;
}

inline bmcl::LogLevel qtToBmclLevel(QtMsgType type)
{
    switch (type)
    {
    case QtDebugMsg: return bmcl::LogLevel::Debug;
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    case QtInfoMsg: return bmcl::LogLevel::Info;
#endif
    case QtWarningMsg: return bmcl::LogLevel::Warning;
    case QtCriticalMsg: return bmcl::LogLevel::Critical;
    case QtFatalMsg:    return bmcl::LogLevel::Panic;
    }
    return bmcl::LogLevel::Debug;
}

void NetProxy::setQtLogHandler()
{
    auto r = [](QtMsgType type, const QMessageLogContext &context, const QString &msg) -> void
    {
        QByteArray utf8 = msg.toUtf8();
        bmcl::LogLevel level = qtToBmclLevel(type);

        bmcl::log(level, utf8.constData());
    };
    qInstallMessageHandler(r);
    _qtLog = true;
    std::cout.flush();
    std::cerr.flush();
}

void NetProxy::setBmclLogHandler()
{
    caf::weak_actor_ptr a;
    if (_logger)
        a = caf::actor_cast<caf::weak_actor_ptr>(_logger);
    auto r = [a](bmcl::LogLevel level, const char* msg)
    {
        auto t = caf::actor_cast<caf::actor>(a);
        if (!t)
        {
            return;
        }
        //auto aid = t->home_system().logger().thread_local_aid();
        caf::anon_send(t, (caf::actor_id)0, level, std::string(msg));
    };
    bmcl::setLogHandler(r);
    _bmclLog = true;
    std::cout.flush();
    std::cerr.flush();
}

}
