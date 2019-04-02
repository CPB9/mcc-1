#pragma once
#include <asio/error_code.hpp>
#include <asio/serial_port.hpp>
#include <fmt/format.h>
#include "mcc/msg/obj/Channel.h"
#include "mcc/net/channels/ChannelImpl.h"


namespace mccnet {

class ChannelCom : public ChannelImpl
{
public:
    ChannelCom(asio::io_service& io_service, ChannelId id, ExchangerPtr&& exch, const mccmsg::ChannelDescription& settings, const mccmsg::NetSerialPtr& params);

protected:
    bool is_open_() const override;
    void connect_(OpCompletion&& f) override;
    void disconnect_(OpCompletion&& f, bmcl::Option<ChannelError>&&) override;
    void data_send_(mccmsg::PacketPtr&& pkt, OpCompletion&& f) override;
    void data_read_() override;
    void update_(const mccmsg::ChannelDescription& settings) override;

private:

    void connect_failed(OpCompletion&& f, const asio::error_code& error, std::string&& msg);
    template<typename T>
    bool set_option(OpCompletion& f, const T option, const char* msg);

    asio::serial_port _socket;
    mccmsg::NetSerialPtr _params;
};

template<typename T>
bool ChannelCom::set_option(OpCompletion& f, const T option, const char* msg)
{
    std::error_code ec;
    _socket.set_option(option, ec);
    if (ec)
        connect_failed(std::move(f), ec, fmt::format("Не удалось задать настройку {} порта", msg));
    return !ec;
}

}
