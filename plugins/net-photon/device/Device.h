#pragma once
#include "mcc/Config.h"
#include "mcc/msg/obj/Channel.h"
#include "mcc/msg/Calibration.h"
#include "mcc/msg/Nav.h"
#include "mcc/msg/ptr/Fwd.h"
#include "mcc/net/Cmd.h"
#include "mcc/net/TmHelper.h"
#include "../Firmware.h"

#include <photon/groundcontrol/GroundControl.h>
#include <caf/event_based_actor.hpp>

#include <memory>

namespace mccphoton {

class Device : public caf::event_based_actor
{
    friend class Visitor;
public:
    Device(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const caf::actor& group,
           const mccmsg::ProtocolId& id, const std::string& name);
    caf::behavior make_behavior() override;
    const char* name() const override;
    void on_exit() override;
private:
    void sendCmd(bmcl::Bytes bytes);
    void sendCmd(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, bmcl::Bytes bytes, bool needPhotonRespHack = false);
    void sendCmd(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const ::photon::PacketRequest& request, bool needPhotonRespHack = false);

    void state_changed();
    void restore_firmware(const mccmsg::ProtocolValue& id);
    void set_firmware(const mccmsg::IFirmwarePtr& frm);

    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdParamList& msg);
    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdCalibrationStart& msg);
    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdCalibrationCancel& msg);
    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdPacketRequest& msg);

    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdParamRead& msg);
    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdParamWrite& msg);

    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteSet& msg);
    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteSetNoActive& msg);
    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteSetActive& msg);
    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteSetActivePoint& msg);
    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteSetDirection& msg);
    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteGet& msg);
    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteGetList& msg);
    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteCreate& msg);
    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteRemove& msg);
    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteClear& msg);

    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdFileGetList& msg);
    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdFileUpload& msg);
    void execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdFileDownload& msg);

    void subscribeForTm();

    struct FrmState
    {
        std::size_t size = 0;
        std::size_t load = 0;
        std::string name;
    };

    FirmwarePtr _pkg;
    FrmState   _frm;
    caf::actor _core;
    caf::actor _broker;
    caf::actor _group;
    caf::actor _gc;
    caf::actor _radioCalibration;

    std::string _name;
    mccmsg::ProtocolId _id;
    mccmsg::StatDevice _stats;
    mccmsg::Motion     _motion;

    mccnet::TmHelper _helper;

    bool _isConnected;
    bmcl::Option<caf::error> _exitReason;
};
}