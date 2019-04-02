#include <bmcl/StringView.h>
#include "mcc/msg/ptr/Device.h"

namespace mccmsg {
namespace device {

ConnectObj::ConnectObj()
{
}

ConnectObj::ConnectObj(const Device& device, const Channel& channel, bool isConnect)
    : _isConnect(isConnect)
    , _device(device)
    , _channel(channel)
{
}

ConnectObj::~ConnectObj()
{
}

ConnectObj::ConnectObj(const ConnectObj& other)
    : _isConnect(other._isConnect)
    , _device(other._device)
    , _channel(other._channel)

{
}

ConnectObj::ConnectObj(ConnectObj&& other)
    : _isConnect(std::move(other._isConnect))
    , _device(std::move(other._device))
    , _channel(std::move(other._channel))

{
}

ConnectObj& ConnectObj::operator=(const ConnectObj& other)
{
    _isConnect = other._isConnect;
    _device = other._device;
    _channel = other._channel;
    return *this;
}

ConnectObj& ConnectObj::operator=(ConnectObj&& other)
{
    _isConnect = std::move(other._isConnect);
    _device = std::move(other._device);
    _channel = std::move(other._channel);
    return *this;
}

ConnectedObj::ConnectedObj(bool isConnected, const Device& device, Channel channel)
    : _isConnected(isConnected)
    , _device(device)
    , _channel(channel)
{
}

ConnectedObj::~ConnectedObj()
{
}

ConnectedObj::ConnectedObj(const ConnectedObj& other)
    : _isConnected(other._isConnected)
    , _device(other._device)
    , _channel(other._channel)

{
}

ConnectedObj::ConnectedObj(ConnectedObj&& other)
    : _isConnected(std::move(other._isConnected))
    , _device(std::move(other._device))
    , _channel(std::move(other._channel))

{
}

ConnectedObj& ConnectedObj::operator=(const ConnectedObj& other)
{
    _isConnected = other._isConnected;
    _device = other._device;
    _channel = other._channel;
    return *this;
}

ConnectedObj& ConnectedObj::operator=(ConnectedObj&& other)
{
    _isConnected = std::move(other._isConnected);
    _device = std::move(other._device);
    _channel = std::move(other._channel);
    return *this;
}

bool ConnectedObj::isConnected() const
{
    return _isConnected;
}
const Device& ConnectedObj::device() const
{
    return _device;
}
const Channel& ConnectedObj::channel() const
{
    return _channel;
}


} // namespace device
} // namespace mccmsg
