#include <algorithm>
#include "mcc/msg/Stats.h"

namespace mccmsg {

Stat::Stat() :_packets(0), _bytes(0), _time(bmcl::SystemClock::now()) {}
Stat::~Stat() {}

void Stat::reset()
{
    _packets = 0;
    _bytes = 0;
    _time = bmcl::SystemClock::now();
}

void Stat::add(std::size_t bytes, std::size_t packets)
{
    if (bytes == 0 && packets == 0)
        return;
    _bytes += bytes;
    _packets += packets;
    _time = bmcl::SystemClock::now();
}

Stat& Stat::operator+=(const Stat& stat)
{
    this->_bytes += stat._bytes;
    this->_packets += stat._packets;
    this->_time = std::max<bmcl::SystemTime>(this->_time, stat._time);
    return *this;
}


Stat::Stat(const Stat& other)
    : _packets(other._packets)
    , _bytes(other._bytes)
    , _time(other._time)
{
}

Stat::Stat(Stat&& other)
    : _packets(std::move(other._packets))
    , _bytes(std::move(other._bytes))
    , _time(std::move(other._time))
{
}

Stat& Stat::operator=(const Stat& other)
{
    _packets = other._packets;
    _bytes = other._bytes;
    _time = other._time;
    return *this;
}

Stat& Stat::operator=(Stat&& other)
{
    _packets = std::move(other._packets);
    _bytes = std::move(other._bytes);
    _time = std::move(other._time);
    return *this;
}

StatChannel::StatChannel() : _isActive(false) {}
StatChannel::StatChannel(const Channel& channel) : _isActive(false), _channel(channel){}
StatChannel::~StatChannel() {}

void StatChannel::reset()
{
    _sent.reset();
    _rcvd.reset();
    _bad.reset();
    _devices.clear();
}

StatChannel& StatChannel::operator+=(const StatChannel& stat)
{
    this->_sent += stat._sent;
    this->_rcvd += stat._rcvd;
    this->_bad  += stat._bad;
    this->_isActive |= stat._isActive;
    return *this;
}

StatChannel::StatChannel(const StatChannel& other)
    : _isActive(other._isActive)
    , _sent(other._sent)
    , _rcvd(other._rcvd)
    , _bad(other._bad)
    , _channel(other._channel)
    , _devices(other._devices)
{
}

StatChannel::StatChannel(StatChannel&& other)
    : _isActive(std::move(other._isActive))
    , _sent(std::move(other._sent))
    , _rcvd(std::move(other._rcvd))
    , _bad(std::move(other._bad))
    , _channel(std::move(other._channel))
    , _devices(std::move(other._devices))
{
}

StatChannel& StatChannel::operator=(const StatChannel& other)
{
    _isActive = other._isActive;
    _sent = other._sent;
    _rcvd = other._rcvd;
    _bad = other._bad;
    _channel = other._channel;
    _devices = other._devices;
    return *this;
}

StatChannel& StatChannel::operator=(StatChannel&& other)
{
    _isActive = std::move(other._isActive);
    _sent = std::move(other._sent);
    _rcvd = std::move(other._rcvd);
    _bad = std::move(other._bad);
    _channel = std::move(other._channel);
    _devices = std::move(other._devices);
    return *this;
}

StatDevice::StatDevice() : _isActive(false) {}
StatDevice::StatDevice(const Device& device) : _isActive(false), _device(device){}
StatDevice::~StatDevice() {}
void StatDevice::reset()
{
    _sent.reset();
    _rcvd.reset();
    _bad.reset();
    _isActive = false;
}
StatDevice& StatDevice::operator+=(const StatDevice& stat)
{
    _sent += stat._sent;
    _rcvd += stat._rcvd;
    _bad += stat._bad;
    return *this;
}
bool StatDevice::isActive() const {return _isActive;}


StatDevice::StatDevice(const StatDevice& other)
    : _isActive(other._isActive)
    , _sent(other._sent)
    , _rcvd(other._rcvd)
    , _bad(other._bad)
    , _device(other._device)
{
}

StatDevice::StatDevice(StatDevice&& other)
    : _isActive(std::move(other._isActive))
    , _sent(std::move(other._sent))
    , _rcvd(std::move(other._rcvd))
    , _bad(std::move(other._bad))
    , _device(std::move(other._device))
{
}

StatDevice& StatDevice::operator=(const StatDevice& other)
{
    _isActive = other._isActive;
    _sent = other._sent;
    _rcvd = other._rcvd;
    _bad = other._bad;
    _device = other._device;
    return *this;
}

StatDevice& StatDevice::operator=(StatDevice&& other)
{
    _isActive = std::move(other._isActive);
    _sent = std::move(other._sent);
    _rcvd = std::move(other._rcvd);
    _bad = std::move(other._bad);
    _device = std::move(other._device);
    return *this;
}
}
