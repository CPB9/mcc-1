#pragma once
#include "mcc/Config.h"
#include <memory>
#include <bmcl/Option.h>
#include "mcc/msg/Packet.h"
#include "mcc/msg/ptr/Fwd.h"
#include "mcc/net/NetLoggerInf.h"

namespace caf { class error; }
namespace caf { class actor; }

namespace mccnet {

struct SearchResult
{
    SearchResult(){}
    explicit SearchResult(std::size_t offset) : _offset(offset){}
    SearchResult(std::size_t offset, std::size_t packet) : _offset(offset), _packet(packet){}
    std::size_t _offset;
    bmcl::Option<std::size_t> _packet;
};

class MCC_PLUGIN_NET_DECLSPEC IExchanger
{
public:
    virtual ~IExchanger() = default;
    virtual SearchResult find(const void * start, std::size_t size) = 0;
    virtual void changeLog(bool state, bool isConnected) = 0;
    virtual void onRcv(const void * start, std::size_t size) = 0;
    virtual void onRcvBad(const void * start, std::size_t size) = 0;
    virtual void onSent(std::size_t req_id, mccmsg::PacketPtr&& pkt, caf::error&& err) = 0;
    virtual void onStats(const mccmsg::StatChannel& stats) = 0;
    virtual void onConnected() = 0;
    virtual void onDisconnected(const caf::error& err) = 0;
};
using ExchangerPtr = std::unique_ptr<IExchanger>;

class MCC_PLUGIN_NET_DECLSPEC DefaultExchanger : public IExchanger
{
public:
    DefaultExchanger(const mccmsg::Channel& channel, const caf::actor& broker, const caf::actor& logger, const LogWriteCreator& creator);
    ~DefaultExchanger();
    void changeLog(bool state, bool isConnected) override;
    void onRcv(const void * start, std::size_t size) override;
    void onRcvBad(const void * start, std::size_t size) override;
    void onSent(std::size_t req_id, mccmsg::PacketPtr&& pkt, caf::error&& err) override;
    void onStats(const mccmsg::StatChannel& stats) override;
    void onConnected() override;
    void onDisconnected(const caf::error& err) override;
    const mccmsg::Channel& channel() const;
    const caf::actor& broker() const;
private:
    struct DefaultExchangerImpl;
    std::unique_ptr<DefaultExchangerImpl> _impl;
};


}
