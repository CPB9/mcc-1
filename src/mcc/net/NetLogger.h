#pragma once
#include <map>
#include <caf/event_based_actor.hpp>
#include "mcc/net/NetLoggerInf.h"

namespace mccnet {

class Logger : public caf::event_based_actor
{
public:
    Logger(caf::actor_config& cfg, bool isConsole);
    void on_exit() override;
    caf::behavior make_behavior() override;
    const char* name() const override;
private:
    void handle(caf::actor_id id, bmcl::LogLevel level, const char* msg, bool hasEndl);
    bool _isConsole;
    std::string _folder;
    std::map<mccmsg::Channel, bmcl::SystemTime> _toOpen;
    std::map<mccmsg::Channel, ILogWriterPtr> _files;
};

}
