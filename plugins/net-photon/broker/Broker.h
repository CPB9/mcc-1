#pragma once
#include <fmt/format.h>
#include <caf/actor.hpp>
#include <caf/event_based_actor.hpp>
#include "mcc/msg/ptr/Tm.h"
#include "mcc/msg/ptr/Protocol.h"

namespace mccphoton {

class Connections;

class Broker : public caf::event_based_actor
{
    friend class Connections;
    friend class NoteVisitorX;
    friend class ReqVisitorX;
public:
    Broker(caf::actor_config& cfg, const caf::actor& core, const caf::actor& logger, const caf::actor& group, const mccmsg::ProtocolDescription& dscr);
    ~Broker();
    caf::behavior make_behavior() override;
    const char* name() const override;
    void on_exit() override;
    std::string getDeviceName(const mccmsg::ProtocolId& id);
private:
    template<typename... A>
    void log(const mccmsg::Device& device, A&&... args)
    {
        send(_core, mccmsg::makeNote(new mccmsg::tm::Log(bmcl::LogLevel::Info, name(), device, fmt::format(std::forward<A>(args)...))));
    }

    void reqChannel(const mccmsg::Channel& name);
    void reqChannels();

    std::string _name;
    caf::actor _core;
    bmcl::Option<caf::error> _exitReason;
    mccmsg::ProtocolDescription _dscr;
    std::unique_ptr<Connections> _conns;
};
}
