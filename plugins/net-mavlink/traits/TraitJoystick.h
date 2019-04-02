#pragma once
#include <caf/atom.hpp>
#include <caf/event_based_actor.hpp>

#include "mcc/net/Cmd.h"
#include "mcc/net/TmHelper.h"

#include "../device/Mavlink.h"
#include "../device/MavlinkUtils.h"

namespace mccmav {

class TraitJoystick : public caf::event_based_actor
{
public:
    TraitJoystick(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const mccmsg::ProtocolId& id, const std::string& name, const MavlinkSettings& settings);
    caf::behavior make_behavior() override;
    const char* name() const override;

    void execute(mccnet::CmdPtr&& cmd, const mccmsg::CmdParamList& ps);
private:
    void joystickTick();

private:
    bool _isActive;
    bool _isEnabled;

    std::string _name;
    caf::actor _core;
    caf::actor _broker;
    mccnet::TmHelper _helper;

    mccmsg::Device _device;
    MavlinkSettings _mavSettings;
    mccmsg::ProtocolId _id;

    double _roll;
    double _pitch;
    double _yaw;
    double _thrust;
};
}
