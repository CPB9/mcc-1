#include <asio/read.hpp>
#include <asio/write.hpp>
#include "mcc/net/channels/ChannelCom.h"
#include "mcc/net/Error.h"

namespace mccnet {

ChannelCom::ChannelCom(asio::io_context& io_context, ChannelId id, ExchangerPtr&& exch, const mccmsg::ChannelDescription& settings, const mccmsg::NetSerialPtr& params)
    : ChannelImpl(io_context, id, std::move(exch), settings)
    , _socket(io_context)
    , _params(params)
{
}

bool ChannelCom::is_open_() const
{
    return _socket.is_open();
}

void ChannelCom::update_(const mccmsg::ChannelDescription& settings)
{
    assert((settings->params()->transport() == mccmsg::NetTransport::Serial));
    auto p = bmcl::dynamic_pointer_cast<const mccmsg::NetSerialParams>(settings->params());
    assert(!p.isNull());
    if (!p.isNull())
        _params = p;
}

void ChannelCom::connect_failed(OpCompletion&& f, const asio::error_code& error, std::string&& msg)
{
    disconnect_(std::move(f), ChannelError(std::move(msg), error));
}

void ChannelCom::connect_(OpCompletion&& f)
{
    if (_socket.is_open())
    {
        ChannelImpl::connected_(std::move(f));
        return;
    }

    std::error_code ec;
    _socket.open(_params->portName(), ec);
    if (ec)
    {
        connect_failed(std::move(f), ec, "Не удалось открыть порт: ");
        return;
    }

    if (!set_option(f, asio::serial_port_base::baud_rate((int)_params->baudRate()), "baud_rate")) return;
    if (!set_option(f, asio::serial_port_base::character_size(8), "character_size")) return;
    if (!set_option(f, asio::serial_port_base::flow_control(asio::serial_port_base::flow_control::none), "flow_control")) return;
    if (!set_option(f, asio::serial_port_base::parity(asio::serial_port_base::parity::none), "parity")) return;
    if (!set_option(f, asio::serial_port_base::stop_bits(asio::serial_port_base::stop_bits::one), "stop_bits")) return;

    ChannelImpl::connected_(std::move(f));
}

void ChannelCom::disconnect_(OpCompletion&& f, bmcl::Option<ChannelError>&& e)
{
    asio::error_code ec;
    _socket.cancel(ec);
    _socket.close(ec);
    ChannelImpl::disconnected_(std::move(f), std::move(e));
}

void ChannelCom::data_send_(mccmsg::PacketPtr&& pkt, OpCompletion&& f)
{
    auto handler = [this, f, pkt](const asio::error_code& error, std::size_t bytes_transferred) mutable { data_sent_(error, pkt, bytes_transferred, std::move(f)); };
    asio::async_write(_socket, asio::buffer(pkt->data(), pkt->size()), std::move(handler));
}

void ChannelCom::data_read_()
{
    auto self = shared_from_this();
    asio::async_read(_socket
                    , asio::dynamic_buffer(_rcvBuf)
                    , [this](const asio::error_code& error, std::size_t bytes_transferred) -> std::size_t { return search_adaptor(error, bytes_transferred); }
                    , [this, self](const asio::error_code& error, std::size_t bytes_transferred) { data_received_(error, bytes_transferred); }
                    );
}

}
