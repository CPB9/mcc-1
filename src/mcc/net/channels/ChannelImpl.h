#pragma once
#include <vector>
#include <atomic>
#include "mcc/net/Asio.h"
#include "mcc/msg/Error.h"
#include "mcc/msg/Stats.h"
#include "mcc/msg/obj/Channel.h"

namespace asio { class io_context; }

namespace mccnet {

class ChannelError
{
public:
    explicit ChannelError(mccmsg::Error e);
    explicit ChannelError(const asio::error_code&);
    ChannelError(bmcl::StringView);
    ChannelError(std::string&&, const asio::error_code&);
    caf::error make_error();
private:
    static std::string to_string(const asio::error_code& e);
    mccmsg::Error _e;
    std::string   _text;
};

class ChannelImpl : public Channel
{
public:
    ChannelImpl(const ChannelImpl&) = delete;
    ChannelImpl& operator=(const ChannelImpl&) = delete;

    ChannelImpl(asio::io_context& io_context, ChannelId id, ExchangerPtr&& exch, const mccmsg::ChannelDescription& settings);
    virtual ~ChannelImpl();

    void send(std::size_t req_id, mccmsg::PacketPtr&& pkt) override final;
    void connect(OpCompletion&& f) override final;
    void disconnect(OpCompletion&& f) override final;
    void update(const mccmsg::ChannelDescription& settings) override final;

protected:

    void setState(bool isActive);
    void connected_(OpCompletion&& f);
    void disconnected_(const OpCompletion& f, bmcl::Option<ChannelError>&&);
    virtual void update_(const mccmsg::ChannelDescription& settings) = 0;

    virtual void connect_(OpCompletion&& f) = 0;
    virtual void disconnect_(OpCompletion&& f, bmcl::Option<ChannelError>&&) = 0;
    virtual void data_send_(mccmsg::PacketPtr&& pkt, OpCompletion&& f) = 0;
    virtual void data_read_() = 0;
    virtual bool is_open_() const = 0;

    void data_sent_(const asio::error_code& error, const mccmsg::PacketPtr& pkt, std::size_t bytes_transferred, OpCompletion&& f);
    void data_received_(const asio::error_code& error, std::size_t bytes_transferred);
    std::size_t search_adaptor(const asio::error_code& error, std::size_t bytes_transferred);

    std::atomic<bool> _rcvStarted;
    std::vector<uint8_t> _rcvBuf;

private:
    inline void addSent(std::size_t bytes, std::size_t packets) { updateStats(_stats._sent, bytes, packets); }
    inline void addRcvd(std::size_t bytes, std::size_t packets) { updateStats(_stats._rcvd, bytes, packets); }
    inline void addBad(std::size_t bytes, std::size_t packets) { updateStats(_stats._bad, bytes, packets); }
    void updateStats(mccmsg::Stat& stat, std::size_t bytes, std::size_t packets);
    void sendStats();
    struct Timer;
    asio::io_context& _io_context;
    mccmsg::StatChannel _stats;
    bmcl::SystemTime _statsSentTime;
    std::shared_ptr<Timer> _reconnectTimer;
    ExchangerPtr  _exch;
};

}
