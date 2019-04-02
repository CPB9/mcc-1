#pragma once
#include "mcc/msg/ptr/Fwd.h"
#include "mcc/msg/Packet.h"
#include "mcc/net/Asio.h"
#include "mcc/net/NetLoggerInf.h"

#include "../broker/Broker.h"
#include "../broker/Device.h"
#include "../broker/Channel.h"

namespace mccmav {

class Connections
{
public:
    Connections(Broker* self, const caf::actor& core, const caf::actor& logger, const caf::actor& group);
    ~Connections();
    bmcl::Option<caf::actor&> getDevice(const mccmsg::Device& name);
    bool hasDevices() const;
    const DItemPtr& addDevice(const mccmsg::ProtocolId& id);
    void deviceDown(const caf::down_msg& dm);
    void removeDevice(const mccmsg::Device& d, const caf::error& reason);
    void removeAllDevices(const caf::error& reason);
    void deviceActivated(const mccmsg::Device& device, bool isActive);
    const DItems& devices() const;

    void sync(const mccmsg::ChannelDescriptions& ds);
    void syncChannel(const mccmsg::ChannelDescription& dscr);
    const CItemPtr& addChannel(const mccmsg::ChannelDescription& dscr);
    bmcl::Option<CItemPtr&> getChannel(const mccmsg::Channel& name);
    void removeChannel(const mccmsg::Channel& name);
    void channelActivate(const mccmsg::channel::Activate_RequestPtr& req, caf::response_promise&& rp);
    void channelActivated(const mccmsg::Channel& channel);
    void channelDeactivated(const mccmsg::Channel& channel, const caf::error& err);

    void connect(const mccmsg::Channel& channel, const mccmsg::ProtocolId& id);
    void disconnect(const mccmsg::Channel& channel, const mccmsg::Device& device);

    void push(const mccmsg::Device& dev, Request&& req);
    void pull(const mccmsg::Channel& c, mccmsg::PacketPtr&& pkt);
    void timeout(const mccmsg::Channel& c);
    mccmsg::StatChannel stats(const mccmsg::Channel& c, const mccmsg::StatChannel& stats);
    bmcl::Option<mccmsg::StatChannel> getStats(const mccmsg::Channel& c);

private:
    mccnet::Asio _asio;
    caf::actor  _core;
    caf::actor  _logger;
    caf::actor  _group;
    Broker*     _self;
    DItems _devices;
    CItems _channels;
};
}