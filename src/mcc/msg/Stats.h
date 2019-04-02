#pragma once
#include "mcc/Config.h"
#include <map>
#include <vector>
#include "mcc/msg/Objects.h"

#include <bmcl/TimeUtils.h>

namespace mccmsg {

class MCC_MSG_DECLSPEC Stat
{
public:
    Stat();
    ~Stat();
    void reset();
    void add(std::size_t bytes, std::size_t packets);
    Stat& operator+=(const Stat& stat);

    Stat(const Stat& other);
    Stat(Stat&& other);
    Stat& operator=(const Stat& other);
    Stat& operator=(Stat&& other);

    std::size_t _packets;
    std::size_t _bytes;
    bmcl::SystemTime _time;
};

class MCC_MSG_DECLSPEC StatChannel
{
public:
    StatChannel();
    explicit StatChannel(const Channel& channel);
    ~StatChannel();
    void reset();
    StatChannel& operator+=(const StatChannel& stat);

    StatChannel(const StatChannel& other);
    StatChannel(StatChannel&& other);
    StatChannel& operator=(const StatChannel& other);
    StatChannel& operator=(StatChannel&& other);

    bool _isActive;
    Stat _sent;
    Stat _rcvd;
    Stat _bad;
    Channel _channel;
    std::map<DeviceId, Stat> _devices;
};
using StatChannels = std::vector<StatChannel>;

class MCC_MSG_DECLSPEC StatDevice
{
public:
    StatDevice();
    explicit StatDevice(const Device& device);
    ~StatDevice();
    void reset();
    StatDevice& operator+=(const StatDevice& stat);
    bool isActive() const;

    StatDevice(const StatDevice& other);
    StatDevice(StatDevice&& other);
    StatDevice& operator=(const StatDevice& other);
    StatDevice& operator=(StatDevice&& other);

    bool _isActive;
    Stat _sent;
    Stat _rcvd;
    Stat _bad;
    Device _device;
};
typedef std::vector<StatDevice> StatDevices;

}

