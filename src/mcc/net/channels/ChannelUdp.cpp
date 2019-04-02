#include <fmt/format.h>
#include "mcc/net/channels/ChannelUdp.h"
#include "mcc/net/Error.h"
#include "mcc/net/NetLoggerInf.h"

namespace mccnet {

ChannelUdp::ChannelUdp(asio::io_service& io_service, ChannelId id, ExchangerPtr&& exch, const mccmsg::ChannelDescription& settings, const mccmsg::NetUdpPtr& params)
    : ChannelImpl(io_service, id, std::move(exch), settings)
    , _socket(io_service)
    , _params(params)
{
    if (_params->remotePort().isNone())
        return;
    //resolve(settings); shared_from_this здесь еще не работает!
}

bool ChannelUdp::is_open_() const
{
    return _socket.is_open();
}

void ChannelUdp::update_(const mccmsg::ChannelDescription& settings)
{
    assert((settings->params()->transport() == mccmsg::NetTransport::Udp));
    auto p = bmcl::dynamic_pointer_cast<const mccmsg::NetUdpParams>(settings->params());
    assert(!p.isNull());
    if (!p.isNull())
        _params = p;
}

void ChannelUdp::connect_(OpCompletion&& f)
{
    if (_socket.is_open())
    {
        ChannelImpl::connected_(std::move(f));
        return;
    }

    if (_params->localPort().isNone() && _params->remotePort().isNone())
    {
        disconnect_(std::move(f), ChannelError("Не заданы ни локальный, ни удалённый порт"));
        return;
    }

    std::error_code ec;
    _socket.open(asio::ip::udp::v4(), ec);
    if (ec)
    {
        disconnect_(std::move(f), ChannelError("Не удалось открыть сокет", ec));
        return;
    }

    if (_params->localPort().isSome())
    {
        _socket.bind(asio::ip::udp::endpoint(asio::ip::udp::v4(), _params->localPort().unwrap()), ec);
        if (ec)
        {
            disconnect_(std::move(f), ChannelError(fmt::format("Не удалось привязать локальный порт {}", _params->localPort().unwrap()), ec));
            return;
        }
    }

    if (_params->remotePort().isNone())
    {
        auto self = shared_from_this();
        auto handler = [this, self, f](const asio::error_code& error) mutable { async_unconnected_data_received_(std::move(f), error); };
        _socket.async_wait(asio::ip::udp::socket::wait_read, handler);
        return;
    }

    asio::ip::address addr = asio::ip::make_address(_params->host(), ec);
    if (ec)
    {
        disconnect_(std::move(f), ChannelError(fmt::format("Некорректный адрес({})", _params->host()), ec));
        return;
    }

    asio::ip::udp::endpoint endpoint(addr, _params->remotePort().unwrap());
    auto self = shared_from_this();
    _socket.async_connect(endpoint, [this, self, f](const asio::error_code& e) mutable { connected_(std::move(f), e); });
}

void ChannelUdp::connected_(OpCompletion&& f, const asio::error_code& error)
{
    if (error)
    {
        disconnect_(std::move(f), ChannelError("Не удалось установить соединение",  error));
        return;
    }
    ChannelImpl::connected_(std::move(f));
}

void ChannelUdp::disconnect_(OpCompletion&& f, bmcl::Option<ChannelError>&& e)
{
    asio::error_code ec;
    //_socket.cancel(ec);
    _socket.shutdown(asio::socket_base::shutdown_both, ec);
    _socket.close(ec);
    ChannelImpl::disconnected_(std::move(f), std::move(e));
}

void ChannelUdp::data_available_(const asio::error_code& error)
{
    if (error)
    {
        data_received_(error, 0);
        return;
    }

    std::size_t start = _rcvBuf.size();
    asio::error_code ec;
    std::size_t bytes_available = _socket.available(ec);

    while (bytes_available)
    {
        std::size_t size = _rcvBuf.size();
        _rcvBuf.resize(size + bytes_available);
        std::size_t bytes = _socket.receive(asio::buffer(_rcvBuf.data() + size, _rcvBuf.size() - size), 0, ec);
        _rcvBuf.resize(size + bytes);
        bytes_available = _socket.available(ec);
    }
    data_received_(error, _rcvBuf.size() - start);
}

void ChannelUdp::async_unconnected_data_received_(OpCompletion&& f, const asio::error_code& error)
{
    if (error && error != asio::error::operation_aborted)
    {
        disconnect_(std::move(f), ChannelError(error));
        return;
    }
    auto self = shared_from_this();
    asio::ip::udp::endpoint endpoint;
    asio::error_code ec;
    _rcvBuf.resize(_socket.available(ec));
    std::size_t bytes = _socket.receive_from(asio::buffer(_rcvBuf), endpoint, 0, ec);
    _rcvBuf.resize(bytes);
    _socket.async_connect(endpoint, [this, self, f](const asio::error_code& error) mutable { connected_(std::move(f), error); });
}

void ChannelUdp::data_send_(mccmsg::PacketPtr&& pkt, OpCompletion&& f)
{
    auto handler = [this, f, pkt](const asio::error_code& error, std::size_t bytes_transferred) mutable { data_sent_(error, pkt, bytes_transferred, std::move(f)); };
    _socket.async_send(asio::buffer(pkt->data(), pkt->size()), std::move(handler));
}

void ChannelUdp::data_read_()
{
    auto self = shared_from_this();
    auto handler = [this, self](const asio::error_code& error) mutable { data_available_(error); };
    _socket.async_wait(asio::ip::udp::socket::wait_read, std::move(handler));
}

}
