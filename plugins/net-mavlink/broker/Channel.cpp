#include <ctime>
#include <caf/send.hpp>
#include <caf/atom.hpp>
#include <bmcl/Logging.h>
#include "mcc/msg/ptr/Channel.h"
#include "mcc/msg/Packet.h"
#include "mcc/net/Asio.h"
#include "mcc/net/NetLoggerInf.h"

#include "../broker/Device.h"
#include "../broker/Channel.h"
#include "../device/Device.h"
#include "../device/Mavlink.h"

MSG_ALLOW_CAF(channel, Activate)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Channel)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NotificationPtr)

namespace mccmav {

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

CItem::CItem(mccnet::Asio& asio, std::unique_ptr<mccnet::IExchanger>&& exchanger, caf::event_based_actor* self, const mccmsg::ChannelDescription& d)
    : _enabled(false), _asio(asio), _self(self), _dscr(d)
{
    _devices = std::make_unique<DevicePrioritizer>();
    _ptr = asio.add(_dscr, std::move(exchanger));
}

CItem::~CItem()
{
    _asio.remove(_ptr->id());
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

inline bool CItem::isEnabled() const
{
    return _enabled;
}

void CItem::activated(bool state)
{
    _enabled = state;

    if (!state && _enabled)
        _ptr->disconnect([](caf::error&&) {});

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

void CItem::connect(const mccmsg::channel::Activate_RequestPtr& req, caf::response_promise&& rp)
{
    auto f = [this, rp, req](caf::error&& e) mutable
    {
        if (e)
            rp.deliver(e);
        else
            rp.deliver(mccmsg::make<mccmsg::channel::Activate_Response>(req.get()));
    };

    _ptr->connect(std::move(f));
}

void CItem::disconnect(const mccmsg::channel::Activate_RequestPtr& req, caf::response_promise&& rp)
{
    auto f = [this, rp, req](caf::error&& e) mutable
    {
        if (e)
            rp.deliver(e);
        else
            rp.deliver(mccmsg::make<mccmsg::channel::Activate_Response>(req.get()));
    };

    _ptr->disconnect(std::move(f));
}

void CItem::update(const mccmsg::ChannelDescription& dscrNew)
{
    if (_dscr->params()->transport() != dscrNew->params()->transport())
        return;

    _dscr = dscrNew;
    _ptr->update(_dscr);
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
    if (pkt->size() <= 4)
        return;
    DeviceId id = MavlinkCoder::device_id(*pkt);
    _stats._devices[id].add(pkt->size(), 1);
    auto r = _devices->get(id);
    if (r.isNone())
        return;
    r.unwrap()->pull(pkt);
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
    _ptr->send(0, std::move(tmp));
    _self->delayed_send(_self, _dscr->timeout(), mccnet::atom_timeout::value, _dscr->name());
}
}
