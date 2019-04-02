#pragma once
#include "mcc/Config.h"
#include <map>
#include <caf/actor.hpp>
#include <caf/event_based_actor.hpp>
#include <bmcl/Option.h>
#include <bmcl/Logging.h>
#include "mcc/Rc.h"
#include "mcc/msg/ptr/Fwd.h"
#include "mcc/msg/Stats.h"
#include "mcc/net/NetPlugin.h"
#include "mcc/plugin/Fwd.h"

namespace mccnet {

class NetLoader : public caf::event_based_actor
{
    friend class NoteVisitorNet;
    friend class ReqVisitorNet;
public:
    NetLoader(caf::actor_config& cfg, const caf::actor& logger);
    ~NetLoader();
    caf::behavior make_behavior() override;
    const char* name() const override;
    void on_exit() override;

private:
    void getPlugins(const bmcl::Rc<const mccplugin::PluginCache>& cache);

    using Protocols = std::map<mccmsg::Protocol, caf::actor>;

    Protocols  _protocols;
    caf::error _exitReason;

private:
    caf::actor _logger;
    caf::actor _db;
    caf::actor _group;
    std::vector<caf::actor> _ui;
    std::map<mccmsg::Device, caf::actor> _devices;
    std::map<mccmsg::Channel, caf::actor> _channels;
};
}
