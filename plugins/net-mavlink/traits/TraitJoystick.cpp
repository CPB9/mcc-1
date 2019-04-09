#include "../traits/Trait.h"
#include "../traits/TraitJoystick.h"
#include "Firmware.h"
#include <utility>

#include <bmcl/Logging.h>
#include <bmcl/MemWriter.h>
#include <bmcl/MakeRc.h>

#include "mcc/net/NetLoggerInf.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmav::MavlinkMessagePtr);

namespace mccmav {

class TraitJoystickVisitor : public mccmsg::CmdVisitor
{
public:
    TraitJoystickVisitor(TraitJoystick* self, mccnet::CmdPtr&& cmd)
        : mccmsg::CmdVisitor([&](const mccmsg::DevReq*) { _cmd->sendFailed(mccmsg::Error::CmdUnknown); })
        , _self(self)
        , _cmd(std::move(cmd))
    {
    }

    using mccmsg::CmdVisitor::visit;

    void visit(const mccmsg::CmdParamList* msg) override { _self->execute(std::move(_cmd), *msg); }
private:
    TraitJoystick* _self;
    mccnet::CmdPtr _cmd;
};

TraitJoystick::TraitJoystick(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const mccmsg::ProtocolId& id, const std::string& name, const MavlinkSettings& mavSettings)
    : caf::event_based_actor(cfg)
    , _isActive(false)
    , _isEnabled(false)
    , _helper(id, core, this)
    , _broker(broker)
    , _name(name)
    , _device(id.device())
    , _mavSettings(mavSettings)
    , _id(id)
    , _roll(0.0)
    , _pitch(0.0)
    , _yaw(0.0)
    , _thrust(0.0)
{
}

const char* TraitJoystick::name() const
{
    return _name.c_str();
}

void TraitJoystick::execute(mccnet::CmdPtr&& cmd, const mccmsg::CmdParamList& ps)
{
    if (ps.command() == "setValues")
    {
        if (ps.params().size() != 4)
        {
            cmd->sendFailed(mccmsg::Error::CmdFailed);
            return;
        }
        _roll = ps.params()[0].toDouble();
        _pitch = ps.params()[1].toDouble();
        _yaw = ps.params()[2].toDouble();
        _thrust = ps.params()[3].toDouble();
        cmd->sendDone();
    }
    if (ps.command() == "setEnabled")
    {
        if (ps.params().size() != 1)
        {
            cmd->sendFailed(mccmsg::Error::CmdFailed);
            return;
        }
        _isEnabled = (ps.params()[0].toInt() == 1);
        std::string text = _isEnabled ? "включено" : "выключено";
        text = "Управление с джойстика " + text;
        _helper.log_text(bmcl::LogLevel::Warning, text);
        cmd->sendDone();
    }
}

void TraitJoystick::joystickTick()
{
    // Store scaling values for all 3 axes
    const float axesScaling = 1.0 * 1000.0;
    // Calculate the new commands for roll, pitch, yaw, and thrust
    const float newRollCommand = _roll * axesScaling;
    // negate pitch value because pitch is negative for pitching forward but mavlink message argument is positive for forward
    const float newPitchCommand = -_pitch * axesScaling;
    const float newYawCommand = _yaw * axesScaling;
    const float newThrustCommand = _thrust * axesScaling;
    quint16 buttons = 0;
    //BMCL_DEBUG() << newRollCommand << newPitchCommand << newYawCommand << newThrustCommand;

    MavlinkMessageRc* msg = new MavlinkMessageRc;
//     // Send the MANUAL_COMMAND message
    mavlink_msg_manual_control_pack_chan(_mavSettings.system,
                                         _mavSettings.component,
                                         _mavSettings.channel,
                                         msg,
                                         _id.id(),
                                         newPitchCommand, newRollCommand, newThrustCommand, newYawCommand, buttons);

    request(_broker, caf::infinite, send_msg_atom::value, MavlinkMessagePtr(msg)).then(
        [this]()
        {
        }
        , [this](const caf::error& err)
        {
        }
    );

}

caf::behavior TraitJoystick::make_behavior()
{
    using timer_atom = caf::atom_constant<caf::atom("timer")>;
    send(this, timer_atom::value);

    return
    {
        [this](activated_atom)
        {
            _isActive = true;
        }
        , [this](deactivated_atom)
        {
            _isActive = false;
        }
        , [this](timer_atom)
        {
            if(_isActive && _isEnabled)
                joystickTick();

            delayed_send(this, std::chrono::milliseconds(25), timer_atom::value);
        }
        , [this](const mccmsg::DevReqPtr& msg)
        {
            TraitJoystickVisitor visitor{ this, bmcl::makeRc<mccnet::Cmd>(make_response_promise(), msg) };
            msg->visit(&visitor);
        }
        , [this](frmupdate_atom, const FirmwarePtr& frm)
        {

        }
    };
}
}
