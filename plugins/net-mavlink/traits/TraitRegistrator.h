#pragma once
#include <unordered_map>
#include <caf/event_based_actor.hpp>
#include "mcc/msg/ptr/Firmware.h"
#include "mcc/msg/ptr/Protocol.h"
#include "../Firmware.h"
#include "../device/Mavlink.h"
#include "../device/MavlinkUtils.h"
#include "mcc/net/TmHelper.h"
#include "mcc/net/Timer.h"

namespace mccmav {

class TraitRegistrator : public caf::event_based_actor
{
    enum class State
    {
        WaitingHeartbeat,
        WaitingVersion,
        WaitingParams,
        Loaded
    };
public:
    TraitRegistrator(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const mccmsg::ProtocolId& id, const std::string& name, const MavlinkSettings& settings);
    caf::behavior make_behavior() override;
    const char* name() const override;

private:
    void on_exit();

    void applyFirmware();
    void removeFirmware();

    void requestAutopilotVersion();
    void requestMissedParams();
    void requestParams();
    void handleParam(const MavlinkMessagePtr& message);
    void readParamByIndex(uint16_t index);

    void update_device(const mccmsg::FirmwareDescription& frm);
    void registered(const mccmsg::FirmwareDescription& tmp);
    void load_frm(std::string&& name);
    void register_frm(mccmsg::FirmwareDescription&& tmp);
    void sendProgress(uint8_t progress);

private:
    mccnet::TmHelper _helper;
    mccmsg::ProtocolId _id;

    std::string _name;
    caf::actor _core;
    caf::actor _broker;
    bool _activated;

    MavlinkSettings _settings;

    State                             _state;
    mccmsg::FirmwareDescription       _mccFirmware;
    std::vector<ParamValue>           _paramsBuffer;

    std::set<uint16_t>                _downloadedParams;
    mccnet::Timer                     _downloadTimer;
    uint16_t                          _paramsCount;
    MAV_AUTOPILOT                     _autopilotKind;
    MAV_TYPE                          _autopilotType;
    std::string                       _fwName;
};
}
