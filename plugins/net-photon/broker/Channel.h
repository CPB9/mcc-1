#pragma once
#include <memory>
#include "mcc/msg/ptr/Fwd.h"
#include "mcc/msg/ptr/Channel.h"
#include "mcc/net/Asio.h"
#include "../broker/Request.h"

namespace mccphoton {

class Broker;
class DItem;
using DItemPtr = mcc::Rc<DItem>;
using DItems = std::map<mccmsg::Device, DItemPtr>;

class IDevicePrioritizer
{
public:
    virtual ~IDevicePrioritizer();
    virtual const std::vector<DItemPtr>& list() const = 0;
    virtual bool empty() const = 0;
    virtual void add(const DItemPtr& d) = 0;
    virtual bmcl::OptionRc<DItem> get(std::size_t id) = 0;
    virtual bmcl::OptionRc<DItem> remove(const mccmsg::Device& d) = 0;
    virtual bmcl::OptionRc<DItem> next() = 0;
};

class CItem : public mcc::RefCountable
{
public:
    CItem(mccnet::Asio& asio, std::unique_ptr<mccnet::IExchanger>&& exchanger, Broker* self, const mccmsg::ChannelDescription& d);
    ~CItem();
    inline std::vector<DItemPtr> devices() const { return _devices->list(); }
    inline const mccmsg::Channel& name() const { return _dscr->name(); }
    void stats(const mccmsg::StatChannel& stats);
    bool isEnabled() const;
    void add_device(const DItemPtr& device);
    void remove_device(const mccmsg::Device& device);
    void activated(bool state);
    void connect(const mccmsg::channel::Activate_RequestPtr& req, caf::response_promise&& rp);
    void disconnect();
    void disconnect(const mccmsg::channel::Activate_RequestPtr& req, caf::response_promise&& rp);
    void update(const mccmsg::ChannelDescription& dscrNew);
    mccmsg::StatChannel getStats();

    void pull(mccmsg::PacketPtr&& pkt);
    void sendIfYouCan();
    void timeout();

private:
    void send(Request&& r);

    Broker* _self;
    mccnet::Asio& _asio;
    mccnet::ChannelPtr ptr;
    //bmcl::Option<Request> _req;
    std::unique_ptr<IDevicePrioritizer> _devices;
    mccmsg::ChannelDescription _dscr;
    bool _pendingTimeout = false;
    mccmsg::StatChannel _stats;
};
using CItemPtr = mcc::Rc<CItem>;
using CItems = std::map<mccmsg::Channel, CItemPtr>;
}
