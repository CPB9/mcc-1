#pragma once
#include "mcc/msg/obj/Channel.h"
#include "mcc/net/channels/ChannelImpl.h"


namespace mccnet {

class ChannelInvalid : public ChannelImpl
{
public:
    ChannelInvalid(asio::io_context& io_context, ChannelId id, ExchangerPtr&& exch, const mccmsg::ChannelDescription& settings);
protected:
    bool is_open_() const override;
    void connect_(OpCompletion&& f) override;
    void disconnect_(OpCompletion&& f, bmcl::Option<ChannelError>&&) override;
    void data_send_(mccmsg::PacketPtr&& pkt, OpCompletion&& f) override;
    void data_read_() override;
    void update_(const mccmsg::ChannelDescription& settings) override;
};

}
