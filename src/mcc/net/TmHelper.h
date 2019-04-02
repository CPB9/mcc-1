#pragma once
#include "mcc/Config.h"
#include <string>
#include <caf/fwd.hpp>
#include <caf/actor.hpp>
#include <bmcl/StringView.h>
#include <bmcl/Logging.h>
#include "mcc/msg/Fwd.h"
#include "mcc/msg/ParamList.h"
#include "mcc/msg/ptr/Protocol.h"

namespace mccnet {

class MCC_PLUGIN_NET_DECLSPEC TmHelper
{
public:
    TmHelper(const mccmsg::ProtocolId& id, const caf::actor& core, caf::event_based_actor* self);
    ~TmHelper();
    const mccmsg::Device& device() const;
    const mccmsg::Protocol& protocol() const;
    const mccmsg::ProtocolId& id() const;
    const caf::actor& core() const;

    void sendStats(const mccmsg::StatDevice&) const;
    void log_text(bmcl::LogLevel logLevel, bmcl::StringView str) const;
    void log(bmcl::LogLevel logLevel, std::string&& str) const;
    void sendTrait(const mccmsg::TmAny*) const;

private:

    mccmsg::ProtocolId _id;
    caf::actor _core;
    caf::event_based_actor* _self;
};

}
