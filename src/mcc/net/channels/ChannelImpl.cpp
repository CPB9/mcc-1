#include <caf/error.hpp>
#include <fmt/format.h>
#include <asio/io_service.hpp>
#include <asio/steady_timer.hpp>
#include <bmcl/Logging.h>
#include "mcc/msg/obj/Channel.h"
#include "mcc/net/Error.h"
#include "mcc/net/channels/ChannelImpl.h"

namespace mccnet {

struct ChannelImpl::Timer
{
    Timer(asio::io_service& s, const bmcl::Option<std::chrono::seconds>& reconnect)
        : _timer(s)
        , _isEnabled(false)
        , _isAuto(reconnect.isSome())
        , _time(reconnect.unwrapOr(std::chrono::seconds(1)))
        , counter(0)
    {
    }
    std::atomic<uint32_t> counter;
    asio::steady_timer _timer;
    std::atomic<bool>  _isEnabled;
    std::atomic<bool>  _isAuto;
    std::atomic<std::chrono::seconds> _time;
};

std::string ChannelError::to_string(const asio::error_code& e)
{
    return QString::fromLocal8Bit(e.message().c_str()).toStdString();
}

ChannelError::ChannelError(mccmsg::Error e) : _e(e)
{
}

ChannelError::ChannelError(const asio::error_code& e) : _e(mccmsg::Error::ChannelError), _text(to_string(e))
{
}

ChannelError::ChannelError(bmcl::StringView text) : _e(mccmsg::Error::ChannelError), _text(text.toStdString())
{
}

ChannelError::ChannelError(std::string&& text, const asio::error_code& e) : _e(mccmsg::Error::ChannelError), _text(fmt::format("{}: ", text, to_string(e)))
{
}

caf::error ChannelError::make_error()
{
    if (_text.empty())
        return  mccmsg::make_error(_e);
    return mccmsg::make_error(_e, std::move(_text));
}

ChannelImpl::ChannelImpl(asio::io_service& io_service, ChannelId id, ExchangerPtr&& exch, const mccmsg::ChannelDescription& settings)
    : Channel(id)
    , _io_service(io_service)
    , _exch(std::move(exch))
    , _stats(settings->name())
{
    _reconnectTimer = std::make_shared<Timer>(io_service, settings->reconnect());
    _rcvStarted = false;
    _exch->changeLog(settings->log(), false);
    setState(false);
}

ChannelImpl::~ChannelImpl()
{
    BMCL_DEBUG() << "Канал обмена удалён из asio: " << _stats._channel.toStdString();
}


void ChannelImpl::send(std::size_t req_id, mccmsg::PacketPtr&& pkt)
{
    auto self = shared_from_this();
    auto handler = [this, req_id, pkt, self](caf::error && err) mutable { _exch->onSent(req_id, std::move(pkt), std::move(err)); };
    _io_service.post([this, handler, pkt]() mutable { data_send_(std::move(pkt), std::move(handler)); });
}

void ChannelImpl::connect(OpCompletion&& f)
{
    auto self = shared_from_this();
    _io_service.post([this, self, f]() mutable { _reconnectTimer->_isEnabled.store(true); connect_(std::move(f)); });
}

void ChannelImpl::disconnect(OpCompletion&& f)
{
    auto self = shared_from_this();
    _io_service.post([this, self, f]() mutable { _reconnectTimer->_isEnabled.store(false); disconnect_(std::move(f), bmcl::None); });
}

void ChannelImpl::update(const mccmsg::ChannelDescription& settings)
{
    auto self = shared_from_this();
    _io_service.post
    (
        [this, self, settings]()
        {
            if (settings->reconnect().isSome())
            {
                _reconnectTimer->_time.store(settings->reconnect().unwrap());
                _reconnectTimer->_isAuto.store(true);
                _reconnectTimer->_isEnabled.store(true);
            }
            else
            {
                _reconnectTimer->_isAuto.store(false);
                _reconnectTimer->_isEnabled.store(false);
            }
            _exch->changeLog(settings->log(), _stats._isActive);
            update_(settings);
        }
    );
}

void ChannelImpl::updateStats(mccmsg::Stat& stat, std::size_t bytes, std::size_t packets)
{
    stat.add(bytes, packets);
    if (bmcl::toMsecs(stat._time - _statsSentTime).count() >= 100)
        sendStats();
}

void ChannelImpl::sendStats()
{
    _exch->onStats(_stats);
    _statsSentTime = bmcl::SystemClock::now();
}

void ChannelImpl::setState(bool isActive)
{
    _stats._isActive = isActive;
    sendStats();
}

void ChannelImpl::data_sent_(const asio::error_code& error, const mccmsg::PacketPtr& pkt, std::size_t bytes_transferred, OpCompletion&& f)
{
    if (error)
        return disconnect_(std::move(f), ChannelError(error));
    if (!is_open_())
        return disconnect_(std::move(f), ChannelError(mccmsg::Error::ChannelClosed));

    assert(bytes_transferred == pkt->size());
    if (bytes_transferred > 0)
        addSent(bytes_transferred, 1);


    f(caf::error());
}

void ChannelImpl::data_received_(const asio::error_code& error, std::size_t bytes_transferred)
{
    (void)bytes_transferred;
    if (error)
        return disconnect_([](caf::error &&){}, ChannelError(error));
    if (!is_open_())
        return disconnect_([](caf::error &&) {}, ChannelError(mccmsg::Error::ChannelClosed));

    bool hasSmth = true;
    std::size_t offset = 0;

    while (!_rcvBuf.empty() && hasSmth)
    {
        SearchResult r = _exch->find(_rcvBuf.data() + offset, _rcvBuf.size() - offset);
        offset += r._offset;
        hasSmth = r._packet.isSome();
        if (!hasSmth)
        {
            std::size_t badSize = r._offset;
            if (badSize > 0)
            {
                addBad(badSize, 0);
                _exch->onRcvBad(_rcvBuf.data() + offset - badSize, badSize);
            }
            continue;
        }
        std::size_t size = *r._packet;
        assert(size > 0);
        {
            addRcvd(size, 1);
            _exch->onRcv(_rcvBuf.data() + offset, size);
        }
        offset += size;
    }

    if (!_rcvBuf.empty())
        _rcvBuf.erase(_rcvBuf.begin(), _rcvBuf.begin() + offset);

    data_read_();
}

void ChannelImpl::connected_(OpCompletion&& f)
{
    if (!is_open_())
        return disconnect_(std::move(f), ChannelError("Неизвестная ошибка при открытии канала"));
    setState(true);
    f(caf::error());
    _exch->onConnected();

    if (!_rcvStarted.exchange(true))
        data_read_();
}

void ChannelImpl::disconnected_(const OpCompletion& f, bmcl::Option<ChannelError>&& e)
{
    setState(false);
    caf::error err;
    if (e.isSome())
        err = e->make_error();

    _exch->onDisconnected(err);
    f(std::move(err));
    _rcvStarted.store(false);

    if (_reconnectTimer->_isEnabled.load() && _reconnectTimer->_isAuto.load())
    {
        asio::error_code ec;
        _reconnectTimer->_timer.expires_from_now(std::chrono::seconds(_reconnectTimer->_time.load()), ec);
        auto self = shared_from_this();
        auto handler = [this, self](const asio::error_code& e) mutable
        {
            if (_reconnectTimer->_isEnabled.load() && _reconnectTimer->_isAuto.load())
                connect_([](caf::error&&){});
        };
        _reconnectTimer->_timer.async_wait(std::move(handler));
    }
}

std::size_t ChannelImpl::search_adaptor(const asio::error_code& error, std::size_t bytes_transferred)
{
    (void)bytes_transferred;
    mccnet::SearchResult r = _exch->find(_rcvBuf.data(), _rcvBuf.size());
    if (error || r._packet.isSome())
        return 0;
    if (r._offset >= 1024)
        return 0;
    return 1024;
}

}
