#pragma once
#include <string>
#include <limits>
#include <array>
#include <deque>

#include <caf/event_based_actor.hpp>

#include <bmcl/MemWriter.h>
#include <bmcl/Result.h>
#include <bmcl/Option.h>
#include <bmcl/Logging.h>


#include "mcc/msg/FwdExt.h"
#include "mcc/msg/Nav.h"
#include "mcc/msg/obj/Channel.h"
#include "mcc/msg/Stats.h"

#include "mcc/net/TmHelper.h"
#include "mcc/net/Timer.h"

#include "mcc/net/Cmd.h"

#include "../Firmware.h"
#include "../device/MavlinkUtils.h"

namespace mccmav {

class Device : public caf::event_based_actor
{
    friend class ParamsVisitor;
public:
    Device(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const caf::actor& group, const mccmsg::ProtocolId & id, const std::string& name);
    caf::behavior make_behavior() override;
    const char* name() const override;
    void on_exit() override;
private:
    template<typename T, typename... A>
    caf::actor createTrait(const char* s, A&&... args);

    void sendMavlinkMessageToChannel(const MavlinkMessagePtr& message);

    void execute(mccnet::CmdPtr&& cmd, mccmsg::CmdParamListPtr&& msg);
    void execute(mccnet::CmdPtr&& cmd, mccmsg::CmdCalibrationStartPtr&& msg);
    void execute(mccnet::CmdPtr&& cmd, mccmsg::CmdCalibrationCancelPtr&& msg);

    void checkSequenceCounter(const MavlinkMessagePtr& message);
    void processMavlinkPacket(mccmsg::PacketPtr&& pkt);

    void sendTm(const MavlinkMessagePtr &message);

    void processMavlinkMessageHeartbeat(const MavlinkMessagePtr& mavlinkMessage);
    void processMavlinkMessageStatusText(const MavlinkMessagePtr& mavlinkMessage);
    void processMavlinkMessageAutopilotVersion(const MavlinkMessagePtr& mavlinkMessage);
    void processMavlinkMessageParamValue(const MavlinkMessagePtr& mavlinkMessage);
    void processMavlinkMessageSysStatus(const MavlinkMessagePtr& mavlinkMessage);
    void processMavlinkMessageAttitude(const MavlinkMessagePtr& mavlinkMessage);
    void processMavlinkMessageGlobalPositionInt(const MavlinkMessagePtr& mavlinkMessage);
    void processMavlinkMessageCommandAck(const MavlinkMessagePtr& mavlinkMessage);
    void processMavlinkMessageGpsRawInt(const MavlinkMessagePtr& mavlinkMessage);
    void processMavlinkMessageRcChannels(const MavlinkMessagePtr& mavlinkMessage);
    void processMavlinkGpsStatus(const MavlinkMessagePtr& mavlinkMessage);

    void state_changed();
    void setActivated(bool state);

    bool _isActive;
    bool _isConnected;

    std::string _name;
    mccmsg::Device _device;
    mccnet::TmHelper _helper;
    mccmsg::ProtocolId _id;

    mccmsg::StatDevice _stats;
    caf::actor _core;
    caf::actor _group;
    caf::actor _broker;
    caf::actor _traitReg;
    caf::actor _traitParams;
    caf::actor _routeController;
    caf::actor _traitCalibration;
    caf::actor _traitRadioCalibration;
    caf::actor _traitJoystick;

    std::vector<caf::actor> _traits;

    uint8_t _targetSystem;
    uint8_t _targetComponent;
    uint8_t _targetChannel;

    mccmsg::Motion _motion;

    void sendCommand(uint16_t cmdId, uint8_t confirmation = 0, float param1 = 0.f, float param2 = 0.f,
                     float param3 = 0.f, float param4 = 0.f, float param5 = 0.f, float param6 = 0.f, float param7 = 0.f);

    void sendCommandWithSystemComponent(uint16_t cmdId, uint8_t targetSystem, uint8_t targetComponent,
                                        uint8_t confirmation = 0, float param1 = 0.f, float param2 = 0.f,
                                        float param3 = 0.f, float param4 = 0.f, float param5 = 0.f, float param6 = 0.f,
                                        float param7 = 0.f);

    bool isPx4() { return true; }

    void setMode(uint8_t mainMode, uint32_t subMode);
    void setArmed(bool armed);
    void setEmergency();
    void takeOff();
    void land();

    void sendStateTm();
private:
    mavlink_heartbeat_t _lastHeartbeat;

    bool _delayedActivation;
    FirmwarePtr _firmware;

    bool _firstPacket;
    uint8_t _lastSequence;
    std::vector<size_t> _activeDevices;
    std::map<std::string, std::string> _traitsMap;
    std::map<int, caf::response_promise> _pendingMessageRequests;
    std::map<int, caf::response_promise> _pendingParamsRequests;
};
}