#pragma once
#include "mcc/Config.h"
#include <memory>
#include <caf/actor.hpp>
#include "mcc/Rc.h"
#include "mcc/plugin/Fwd.h"
#include "mcc/msg/Fwd.h"

namespace caf { class actor; }
namespace caf { class actor_system; }
namespace caf { class actor_system_config; }

namespace mccnet {

class MCC_NET_DECLSPEC NetProxy : public mcc::RefCountable
{
public:
    explicit NetProxy(bool isConsole, bmcl::Option<uint16_t> maxThreads, mccplugin::PluginCache* cache, bmcl::Option<uint16_t> port);
    explicit NetProxy(bool isConsole, bmcl::Option<uint16_t> maxThreads, mccplugin::PluginCache* cache, const std::string& host, uint16_t port);
    NetProxy(const NetProxy &) = delete;
    ~NetProxy() override;

    const caf::actor& core() const;
    const caf::actor& logger() const;
    const caf::actor_system& actor_system() const;


private:
    void setQtLogHandler();
    void setBmclLogHandler();
    void loadPlugins(mccplugin::PluginCache* cache);

    bool _internal;
    bool _qtLog;
    bool _bmclLog;
    caf::actor _logger;
    caf::actor _loader;
    std::unique_ptr<caf::actor_system> _sys;
    std::unique_ptr<caf::actor_system_config> _cfg;
};

}
