#pragma once
#include <map>
#include <caf/atom.hpp>
#include <caf/event_based_actor.hpp>

#include <bmcl/OptionRc.h>
#include <bmcl/TimeUtils.h>

#include "mcc/msg/ptr/Fwd.h"
#include "mcc/net/Cmd.h"
#include "mcc/net/TmHelper.h"

#include "../device/Mavlink.h"
#include "../device/MavlinkUtils.h"
#include "../Firmware.h"

namespace mccmav {

class TraitParams : public caf::event_based_actor
{
    friend class TraitParamsCmdVisitor;

public:
    TraitParams(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const mccmsg::ProtocolId& id, const std::string& name, const MavlinkSettings& settings);
    caf::behavior make_behavior() override;
    const char* name() const override;

private:
    void processMavlinkMessage(const mavlink_param_value_t& msg);

    void writeParam(const std::string& id, const mccmsg::NetVariant& value);
    void readParam(const std::string& id, const mccmsg::NetVariant& value);

    void execute(mccnet::CmdPtr&& cmd, const mccmsg::CmdParamReadPtr& msg);
    void execute(mccnet::CmdPtr&& cmd, const mccmsg::CmdParamWritePtr& msg);
private:
    mccnet::TmHelper _helper;
    mccmsg::ProtocolId _id;
    std::string _name;
    caf::actor _core;
    caf::actor _broker;

    bmcl::OptionRc<const Firmware> _firmware;

    bool _activated;

    MavlinkSettings _settings;

    std::vector<ParamValue> _params;
    std::vector<mccnet::CmdPtr>     _queuedCmds;
};
}