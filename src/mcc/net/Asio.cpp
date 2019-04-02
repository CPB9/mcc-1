#include <thread>

#include <asio/io_service.hpp>
#include <asio/error_code.hpp>
#include <asio/thread_pool.hpp>

#include <bmcl/MakeRc.h>

#include "mcc/msg/ptr/Fwd.h"
#include "mcc/msg/obj/Channel.h"
#include "mcc/net/Error.h"
#include "mcc/net/Asio.h"
#include "mcc/net/channels/ChannelUdp.h"
#include "mcc/net/channels/ChannelTcp.h"
#include "mcc/net/channels/ChannelCom.h"
#include "mcc/net/channels/ChannelInvalid.h"

namespace mccnet {

class Asio::AsioData
{
public:
    AsioData()
    {
    }
    ~AsioData()
    {
        stop();
    }
    void start()
    {
        if (_work)
            return;
        _work = std::make_unique<asio::io_service::work>(_io_service);
        _thread = std::thread(std::move([&] { _io_service.run(); }));
    }
    void stop()
    {
        if (!_work)
            return;
        _work.reset();
        _io_service.stop();
        _thread.join();
        assert(_io_service.stopped());
    }
    inline asio::io_service& io_service() { return _io_service; };
private:
    asio::io_service _io_service;
    std::thread _thread;
    std::unique_ptr<asio::io_service::work> _work;
};

Asio::Asio()
{
    _asio = std::make_unique<AsioData>();
    start();
}

Asio::~Asio()
{
    stop();
}

void Asio::start()
{
    _asio->start();
}

void Asio::stop()
{
    for (auto& i : _channels)
        i.second->disconnect([](const caf::error&) {});
    if (_asio)
        _asio->stop();
    _channels.clear();
    _asio.reset();
}

void Asio::remove(ChannelId channel)
{
    const auto& i = _channels.find(channel);
    if (i != _channels.end())
    {
        i->second->disconnect([](const caf::error&) {});
        _channels.erase(i);
    }
}

ChannelPtr Asio::add(const mccmsg::ChannelDescription& description, ExchangerPtr&& exch)
{
    auto r = makeChannel(std::move(exch), description);
    if (!r)
        return nullptr;
    auto id = r->id();
    return _channels.emplace(id, std::move(r)).first->second;
}

bmcl::Option<ChannelPtr&> Asio::get(ChannelId channel)
{
    auto i = _channels.find(channel);
    if (i == _channels.end())
        return bmcl::None;
    return i->second;
}

std::set<ChannelId> Asio::list() const
{
    std::set<ChannelId> cs;
    for (const auto& i : _channels)
        cs.emplace(i.first);
    return cs;
}

class NetTrVisitor : public mccmsg::INetVisitor
{
public:
    NetTrVisitor(asio::io_service& io, std::size_t id, ExchangerPtr&& exch, const mccmsg::ChannelDescription& d) : _id(id), _exch(std::move(exch)), _d(d), _io(io){}
    void visit(const mccmsg::NetTcpParams& params) override { _channel = std::make_shared<ChannelTcp>(_io, _id, std::move(_exch), _d, bmcl::wrapRc(&params));}
    void visit(const mccmsg::NetUdpParams& params) override { _channel = std::make_shared<ChannelUdp>(_io, _id, std::move(_exch), _d, bmcl::wrapRc(&params)); }
    void visit(const mccmsg::NetSerialParams& params) override { _channel = std::make_shared<ChannelCom>(_io, _id, std::move(_exch), _d, bmcl::wrapRc(&params)); }
    ChannelPtr channel() { return _channel; }
private:
    std::size_t     _id;
    ChannelPtr      _channel;
    ExchangerPtr    _exch;
    mccmsg::ChannelDescription _d;
    asio::io_service& _io;
};

ChannelPtr Asio::makeChannel(ExchangerPtr&& exch, const mccmsg::ChannelDescription& d)
{
    NetTrVisitor visitor(_asio->io_service(), _counter++, std::move(exch), d);
    d->params()->visit(visitor);
    return visitor.channel();
}

}
