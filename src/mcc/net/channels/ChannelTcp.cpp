#include <asio/read.hpp>
#include <asio/write.hpp>
#include <fmt/format.h>
#include "mcc/net/channels/ChannelTcp.h"
#include "mcc/net/Error.h"

namespace mccnet {

ChannelTcp::ChannelTcp(asio::io_service& io_service, ChannelId id, ExchangerPtr&& exch, const mccmsg::ChannelDescription& settings, const mccmsg::NetTcpPtr& params)
    : ChannelImpl(io_service, id, std::move(exch), settings)
    , _socket(io_service)
    , _params(params)
{
}

bool ChannelTcp::is_open_() const
{
    return _socket.is_open();
}

void ChannelTcp::update_(const mccmsg::ChannelDescription& settings)
{
    assert((settings->params()->transport() == mccmsg::NetTransport::Tcp));
    auto p = bmcl::dynamic_pointer_cast<const mccmsg::NetTcpParams>(settings->params());
    assert(!p.isNull());
    if (!p.isNull())
        _params = p;
}

void ChannelTcp::connect_(OpCompletion&& f)
{
    if (_socket.is_open())
    {
        ChannelImpl::connected_(std::move(f));
        return;
    }

    asio::error_code ec;
    asio::ip::address addr = asio::ip::make_address(_params->host(), ec);
    if (ec)
    {
        disconnect_(std::move(f), ChannelError(fmt::format("Некорректный адрес({}): {}", _params->host()), ec));
        return;
    }

    asio::ip::tcp::endpoint endpoint(addr, _params->remotePort());
    auto self = shared_from_this();
    auto handler = [this, self, f](const asio::error_code& error) mutable {connected_(std::move(f), error);};
    _socket.async_connect(endpoint, std::move(handler));
}

void ChannelTcp::connected_(OpCompletion&& f, const std::error_code& ec)
{
    if (ec)
    {
        disconnect_(std::move(f), ChannelError("Не удалось открыть сокет", ec));
        return;
    }

    asio::error_code e;
    _socket.set_option(asio::socket_base::keep_alive(true), e);
    if (e)
    {
        disconnect_(std::move(f), ChannelError("Не удалось задать keep_alive", e));
        return;
    }

    _socket.set_option(asio::ip::tcp::no_delay(true), e);
    if (e)
    {
        disconnect_(std::move(f), ChannelError("Не удалось задать no_delay", e));
        return;
    }

    ChannelImpl::connected_(std::move(f));
}

void ChannelTcp::disconnect_(OpCompletion&& f, bmcl::Option<ChannelError>&& e)
{
    asio::error_code ec;
    _socket.cancel(ec);
    _socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    _socket.close(ec);
    ChannelImpl::disconnected_(std::move(f), std::move(e));
}

void ChannelTcp::data_send_(mccmsg::PacketPtr&& pkt, OpCompletion&& f)
{
    auto handler = [this, f, pkt](const asio::error_code& error, std::size_t bytes_transferred) mutable { data_sent_(error, pkt, bytes_transferred, std::move(f)); };
    asio::async_write(_socket, asio::buffer(pkt->data(), pkt->size()), handler);
}

void ChannelTcp::data_read_()
{
    auto self = shared_from_this();
    asio::async_read(_socket
                     , asio::dynamic_buffer(_rcvBuf)
                     , [this](const asio::error_code& error, std::size_t bytes_transferred) -> std::size_t { return search_adaptor(error, bytes_transferred); }
                     , [this, self](const asio::error_code& error, std::size_t bytes_transferred) { data_received_(error, bytes_transferred); }
                    );
}

}
