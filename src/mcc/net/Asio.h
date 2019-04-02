#pragma once
#include "mcc/Config.h"

#include <memory>
#include <functional>
#include <map>
#include <set>
#include "mcc/msg/obj/Channel.h"
#include "mcc/msg/Packet.h"
#include "mcc/net/Exchanger.h"

namespace mccnet {

using ChannelId = std::size_t;
using OpCompletion = std::function<void(caf::error&& err)>;

class MCC_PLUGIN_NET_DECLSPEC Channel : public std::enable_shared_from_this<Channel>
{
public:
    explicit Channel(ChannelId id) : _id(id) {}
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;
    virtual ~Channel() = default;
    inline ChannelId id() const { return _id; }

    virtual void send(std::size_t req_id, mccmsg::PacketPtr&& pkt) = 0;
    virtual void connect(OpCompletion&& f) = 0;
    virtual void disconnect(OpCompletion&& f) = 0;
    virtual void update(const mccmsg::ChannelDescription& settings) = 0;
private:
    ChannelId _id;
};
using ChannelPtr = std::shared_ptr<Channel>;

class MCC_PLUGIN_NET_DECLSPEC Asio
{
public:
    Asio();
    ~Asio();
    std::set<ChannelId> list() const;
    bmcl::Option<ChannelPtr&> get(ChannelId channel);
    void remove(ChannelId channel);
    ChannelPtr add(const mccmsg::ChannelDescription& description, ExchangerPtr&& exch);

    void start();
    void stop();

private:
    ChannelId _counter = 0;
    class AsioData;
    ChannelPtr makeChannel(ExchangerPtr&& exch, const mccmsg::ChannelDescription& description);

    std::unique_ptr<AsioData> _asio;
    std::map<ChannelId, ChannelPtr> _channels;
};

}
