#pragma once
#include <asio/ip/tcp.hpp>
#include "mcc/msg/obj/Channel.h"
#include "mcc/net/channels/ChannelImpl.h"


namespace mccnet {

class ChannelTcp : public ChannelImpl
{
public:
    ChannelTcp(asio::io_service& io_service, ChannelId id, ExchangerPtr&& exch, const mccmsg::ChannelDescription& settings, const mccmsg::NetTcpPtr& params);

protected:
    bool is_open_() const override;
    void connect_(OpCompletion&& f) override;
    void disconnect_(OpCompletion&& f, bmcl::Option<ChannelError>&&) override;
    void data_send_(mccmsg::PacketPtr&& pkt, OpCompletion&& f) override;
    void data_read_() override;
    void update_(const mccmsg::ChannelDescription& settings) override;

private:
    void connected_(OpCompletion&& f, const asio::error_code& error);

    asio::ip::tcp::socket _socket;
    mccmsg::NetTcpPtr _params;
};

}
