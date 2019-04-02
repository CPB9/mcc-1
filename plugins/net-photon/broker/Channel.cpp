#include <caf/actor.hpp>
#include <caf/event_based_actor.hpp>
#include <photon/groundcontrol/GroundControl.h>
#include <bmcl/Utils.h>
#include <bmcl/Logging.h>
#include <bmcl/OptionRc.h>
#include "mcc/msg/ptr/Channel.h"
#include "mcc/net/NetLoggerInf.h"
#include "../broker/Channel.h"
#include "../broker/Device.h"
#include "../broker/Broker.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Channel)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::channel::Activate_ResponsePtr)

namespace mccphoton {

IDevicePrioritizer::~IDevicePrioritizer()
{
}

class DevicePrioritizer : public IDevicePrioritizer
{
public:
    const std::vector<DItemPtr>& list() const override { return _devices; }
    bool empty() const override { return _devices.empty(); }
    void add(const DItemPtr& d) override
    {
        auto i = std::find_if(_devices.begin(), _devices.end(), [d](const DItemPtr& item) { return d->name() == item->name(); });
        if (i == _devices.end())
            _devices.emplace_back(d);
    }
    bmcl::OptionRc<DItem> remove(const mccmsg::Device& d) override
    {
        auto i = std::find_if(_devices.begin(), _devices.end(), [d](const DItemPtr& item) { return d == item->name(); });
        if (i == _devices.end())
            return bmcl::None;
        if (std::distance(_devices.begin(), i) <= _pos && _pos > 0)
            --_pos;
        DItemPtr dev = *i;
        _devices.erase(i);
        return dev;
    }
    bmcl::OptionRc<DItem> get(std::size_t id) override
    {
        auto i = std::find_if(_devices.begin(), _devices.end(), [id](const DItemPtr& item) { return item->isSameId(id); });
        if (i == _devices.end())
            return bmcl::None;
        return *i;
    }
    bmcl::OptionRc<DItem> next() override
    {
        if (_devices.empty())
            return bmcl::None;
        std::size_t size = _devices.size();
        for (std::size_t i = 1; i <= size; ++i)
        {
            const DItemPtr& item = _devices[(_pos + i) % size];
            if (item->hasRequests())
            {
                _pos = (_pos + i) % size;
                return item;
            }
        }
        return bmcl::None;
    }
private:
    std::size_t _pos = 0;
    std::vector<DItemPtr> _devices;
};

CItem::CItem(mccnet::Asio& asio, std::unique_ptr<mccnet::IExchanger>&& exchanger, Broker* self, const mccmsg::ChannelDescription& d)
    : _asio(asio), _self(self), _dscr(d)
{
    _stats._isActive = false;
    _devices = std::make_unique<DevicePrioritizer>();
    ptr = asio.add(_dscr, std::move(exchanger));
    ptr->update(d); //чтобы вызывался resolve!
}

CItem::~CItem()
{
    _asio.remove(ptr->id());
    BMCL_DEBUG() << "Канал удалён из брокера: " << _dscr->name().toStdString();
}

void CItem::add_device(const DItemPtr& device)
{
    _devices->add(device);
    if (isEnabled())
        device->addChannel(this);
}

void CItem::remove_device(const mccmsg::Device& device)
{
    auto d = _devices->remove(device);
    if (d.isSome())
        d.unwrap()->removeChannel(name());
}

bool CItem::isEnabled() const
{
    return _stats._isActive;
}

void CItem::activated(bool state)
{
    _stats._isActive = state;
    if (!state && isEnabled())
        ptr->disconnect([](caf::error&&) {});

    auto ds = _devices->list();
    for (const auto& j : ds)
    {
        if (isEnabled())
            j->addChannel(this);
        else
            j->removeChannel(name());
    }

    if (isEnabled())
        sendIfYouCan();

    if (!isEnabled())
        _stats.reset();
}

void CItem::connect(const mccmsg::channel::Activate_RequestPtr& r, caf::response_promise&& rp)
{
    auto f = [this, rp, r](caf::error&& e) mutable
    {
        if ((bool)e)
            rp.deliver(e);
        else
            rp.deliver(mccmsg::make<mccmsg::channel::Activate_Response>(r.get()));
    };
    ptr->connect(std::move(f));
}

void CItem::disconnect()
{
    ptr->disconnect([](caf::error&& e) {});
}

void CItem::disconnect(const mccmsg::channel::Activate_RequestPtr& r, caf::response_promise&& rp)
{
    auto f = [this, rp, r](caf::error&& e) mutable
    {
        if ((bool)e)
            rp.deliver(e);
        else
            rp.deliver(mccmsg::make<mccmsg::channel::Activate_Response>(r.get()));
    };

    ptr->disconnect(std::move(f));
}

void CItem::update(const mccmsg::ChannelDescription& dscrNew)
{
    if (_dscr->params()->transport() != dscrNew->params()->transport())
        return;
    _dscr = dscrNew;
    ptr->update(_dscr);
}

void CItem::timeout()
{
    _pendingTimeout = false;
    sendIfYouCan();
}

void CItem::sendIfYouCan()
{
    if (!isEnabled() || _pendingTimeout)
        return;
    auto d = _devices->next();
    if (d.isNone())
        return;
    send((*d)->popRequest());
}

void CItem::pull(mccmsg::PacketPtr&& pkt)
{
    auto list = _devices->list();
    if (list.empty())
        return;

    bmcl::Option<::photon::PacketAddress> addr = ::photon::GroundControl::extractPacketAddress(bmcl::Bytes(*pkt));
    if (addr.isNone())
    {
        assert(false);
        return;
    }

    uint64_t id = addr->srcAddress;
    _stats._devices[id].add(pkt->size(), 1);

    auto d = _devices->get(id);
    if (d.isNone())
    {
        //неизвестное устройство
        return;
    }
    d.unwrap()->pull(pkt);
}

void CItem::stats(const mccmsg::StatChannel& stats)
{
    _stats._sent = stats._sent;
    _stats._bad = stats._bad;
    _stats._rcvd = stats._rcvd;
}

mccmsg::StatChannel CItem::getStats()
{
    mccmsg::StatChannel s = _stats;
    s._channel = _dscr->name();
    s._isActive = isEnabled();
    return s;
}

void CItem::send(Request&& r)
{
    _pendingTimeout = true;
    auto tmp = r.pkt;
    ptr->send(0, std::move(tmp));
    _self->delayed_send(_self, _dscr->timeout(), mccnet::atom_timeout::value, _dscr->name());
}
}
