#include <caf/send.hpp>
#include <caf/atom.hpp>
#include <bmcl/Logging.h>
#include <bmcl/Utils.h>
#include "mcc/msg/ptr/Channel.h"
#include "mcc/msg/ptr/Tm.h"
#include "mcc/msg/Packet.h"
#include "mcc/net/Asio.h"
#include "mcc/net/NetLoggerInf.h"
#include "../broker/Broker.h"
#include "../broker/Exchanger.h"
#include "../broker/Connections.h"
#include "../device/Device.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::PacketPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Channel);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Device);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NotificationPtr);

namespace mccphoton {

Connections::Connections(Broker* self, const caf::actor& core, const caf::actor& logger, const caf::actor& group)
    : _self(self), _core(core), _logger(logger), _group(group)
{
}

Connections::~Connections()
{
}

bmcl::Option<caf::actor&> Connections::getDevice(const mccmsg::Device& name)
{
    const auto& i = _devices.find(name);
    if (i == _devices.end())
        return bmcl::None;
    return i->second->actor();
}

bool Connections::hasDevices() const
{
    return !_devices.empty();
}

const DItemPtr& Connections::addDevice(const mccmsg::ProtocolId& id)
{
    mccmsg::Device name = id.device();
    auto i = _devices.find(name);
    if (i == _devices.end())
    {
        auto a = _self->spawn<Device, caf::monitored>(_core, _self, _group, id, _self->getDeviceName(id));
        auto j = _devices.emplace(name, new DItem(id, a, _self));
        _self->send(_core, caf::atom("device"), id.device());
        i = j.first;
    }
    return i->second;
}

void Connections::deviceDown(const caf::down_msg& dm)
{
    caf::actor_addr addr = dm.source;
    auto i = std::find_if(_devices.begin(), _devices.end(), [addr](const DItems::value_type& i) { return i.second->isSame(addr); });
    if (i == _devices.end())
        return;
    assert(i != _devices.end());
    if (dm.reason == caf::sec::runtime_error)
        _self->log(i->second->name(), "актор устройства остановлен из-за ошибки");
    else
        _self->log(i->second->name(), "актор устройства остановлен по причине: {}", _self->system().render(dm.reason));
    for (const auto& j : _channels)
        j.second->remove_device(i->second->name());
    _devices.erase(i);
}

void Connections::removeDevice(const mccmsg::Device& d, const caf::error& reason)
{
    auto i = _devices.find(d);
    if (i == _devices.end())
        return;
    _self->send_exit(i->second->actor(), reason);
}

void Connections::removeAllDevices(const caf::error& reason)
{
    for (const auto& i : _devices)
        _self->send_exit(i.second->actor(), reason);
}

const DItems& Connections::devices() const
{
    return _devices;
}

void Connections::syncChannel(const mccmsg::ChannelDescription& dscr)
{
    for (const auto& i : dscr->connectedDevices())
    {
        connect(dscr->name(), i);
    }

    auto i = _channels.find(dscr->name());
    if (i == _channels.end())
    {
        assert(false);
        return;
    }
    auto& c = i->second;
    const auto& current = c->devices();
    for (const DItemPtr& d : current)
    {
        auto j = std::find_if(dscr->connectedDevices().begin(), dscr->connectedDevices().end(), [d](const mccmsg::ProtocolId& id) { return id.device() == d->name(); });
        if (j == dscr->connectedDevices().end())
            disconnect(dscr->name(), d->name());
    }
    c->update(dscr);
}

const CItemPtr& Connections::addChannel(const mccmsg::ChannelDescription& dscr)
{
    auto i = _channels.find(dscr->name());
    if (i == _channels.end())
    {
        auto j = _channels.emplace(dscr->name(), new CItem(_asio, std::make_unique<Exchanger>(dscr->name(), _self, _logger), _self, dscr));
        i = j.first;
    }

    syncChannel(dscr);
    send_as(_self, _core, caf::atom("channel"), dscr->name());
    i->second->sendIfYouCan();
    return i->second;
}

bmcl::Option<CItemPtr&> Connections::getChannel(const mccmsg::Channel& name)
{
    auto i = _channels.find(name);
    if (i == _channels.end())
        return bmcl::None;
    return i->second;
}

void Connections::removeChannel(const mccmsg::Channel& name)
{
    _channels.erase(name);
    for (const auto& i : _devices)
        i.second->removeChannel(name);
}

void Connections::sync(const mccmsg::ChannelDescriptions& ds)
{
    std::set<mccmsg::Channel> cs;
    for (const auto& i : ds)
    {
        cs.emplace(i->name());
        addChannel(i);
    }

    auto i = _channels.begin();
    for (; i != _channels.end(); )
    {
        if (cs.find(i->first) != cs.end())
            ++i;
        else
        {
            _channels.erase(i++);
        }
    }
}

void Connections::channelActivate(const mccmsg::channel::Activate_RequestPtr& req, caf::response_promise&& rp)
{
    const auto i = _channels.find(req->data()._name);
    if (i == _channels.end())
    {
        rp.deliver(mccmsg::make_error(mccmsg::Error::ChannelUnknown));
        return;
    }

    if (!req->data()._state)
    {
        i->second->disconnect(req, std::move(rp));
        return;
    }

    for (auto& j : _channels)
    {
        if (j.first != req->data()._name)
            j.second->disconnect();
    }
    i->second->connect(req, std::move(rp));
}

void Connections::channelActivated(const mccmsg::Channel& channel)
{
    const auto i = _channels.find(channel);
    if (i == _channels.end())
        return;
    i->second->activated(true);
}

void Connections::channelDeactivated(const mccmsg::Channel& channel, const caf::error& err)
{
    const auto i = _channels.find(channel);
    if (i == _channels.end())
        return;
    if (err)
    {
        BMCL_WARNING() << "Ошибка соединения: " << _self->system().render(err);
    }
    i->second->activated(false);
}

void Connections::connect(const mccmsg::Channel& channel, const mccmsg::ProtocolId& id)
{
    auto i = _channels.find(channel);
    if (i == _channels.end())
    {
        assert(false);
        return;
    }
    auto& c = i->second;
    c->add_device(addDevice(id));
}

void Connections::disconnect(const mccmsg::Channel& channel, const mccmsg::Device& device)
{
    auto i = _channels.find(channel);
    if (i == _channels.end())
        return;
    i->second->remove_device(device);

    auto j = _devices.find(device);
    if (j == _devices.end())
    {
        assert(false);
        return;
    }
    j->second->removeChannel(channel);
}

void Connections::push(const mccmsg::Device& dev, Request&& req)
{
    auto i = _devices.find(dev);
    if (i == _devices.end())
    {
        BMCL_WARNING() << fmt::format("устройство {} преждевременно удалено из обменки!", dev.toStdString());
        return;
    }
    i->second->pushRequest(std::move(req));
}

void Connections::pull(const mccmsg::Channel& c, mccmsg::PacketPtr&& pkt)
{
    auto i = _channels.find(c);
    if (i == _channels.end())
        return;
    i->second->pull(std::move(pkt));
}

void Connections::timeout(const mccmsg::Channel& c)
{
    auto i = _channels.find(c);
    if (i == _channels.end())
        return;
    i->second->timeout();
}

mccmsg::StatChannel Connections::stats(const mccmsg::Channel& c, const mccmsg::StatChannel& stats)
{
    auto i = _channels.find(c);
    if (i == _channels.end())
        return stats;
    i->second->stats(stats);
    return i->second->getStats();
}

bmcl::Option<mccmsg::StatChannel> Connections::getStats(const mccmsg::Channel& c)
{
    auto i = _channels.find(c);
    if (i == _channels.end())
        return bmcl::None;
    return i->second->getStats();
}
}