#pragma once
#include "mcc/Config.h"
#include <memory>
#include "mcc/Rc.h"
#include "mcc/plugin/Fwd.h"
#include "mcc/msg/Fwd.h"

namespace caf { class actor; }
namespace caf { class actor_system; }
namespace caf { class actor_system_config; }

namespace mccnet {

class MCC_NET_DECLSPEC NetProxy
{
public:
    explicit NetProxy(bool isConsole, bmcl::Option<uint16_t> maxThreads);
    NetProxy(const NetProxy &) = delete;
    ~NetProxy();

    void loadPlugins(mccplugin::PluginCache* cache);

    const caf::actor& core() const;
    const caf::actor& logger() const;
    const caf::actor_system& actor_system() const;

    void setQtLogHandler();
    void setBmclLogHandler();

private:
    bool _qtLog;
    bool _bmclLog;
    std::unique_ptr<caf::actor> _logger;
    std::unique_ptr<caf::actor> _loader;
    std::unique_ptr<caf::actor_system> _sys;
    std::unique_ptr<caf::actor_system_config> _cfg;
};

}
