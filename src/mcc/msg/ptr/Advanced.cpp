#include "mcc/msg/ptr/Advanced.h"

namespace mccmsg {
namespace advanced {


ChannelAndDeviceReqObj::ChannelAndDeviceReqObj() {}
ChannelAndDeviceReqObj::ChannelAndDeviceReqObj(const DeviceDescription& d, const ChannelDescription& c)
    : device(d), channel(c) {}
ChannelAndDeviceReqObj::~ChannelAndDeviceReqObj() {}

ChannelAndDeviceReqObj::ChannelAndDeviceReqObj(const ChannelAndDeviceReqObj& other)
    : device(other.device)
    , channel(other.channel)
{
}

ChannelAndDeviceReqObj::ChannelAndDeviceReqObj(ChannelAndDeviceReqObj&& other)
    : device(std::move(other.device))
    , channel(std::move(other.channel))
{
}

ChannelAndDeviceReqObj& ChannelAndDeviceReqObj::operator=(const ChannelAndDeviceReqObj& other)
{
    device = other.device;
    channel = other.channel;
    return *this;
}

ChannelAndDeviceReqObj& ChannelAndDeviceReqObj::operator=(ChannelAndDeviceReqObj&& other)
{
    device = std::move(other.device);
    channel = std::move(other.channel);
    return *this;
}

ChannelAndDeviceRepObj::ChannelAndDeviceRepObj() {}
ChannelAndDeviceRepObj::ChannelAndDeviceRepObj(const ProtocolId& id, const Channel& channel)
    : id(id), channel(channel) {}
ChannelAndDeviceRepObj::~ChannelAndDeviceRepObj() {}

ChannelAndDeviceRepObj::ChannelAndDeviceRepObj(const ChannelAndDeviceRepObj& other)
    : id(other.id)
    , channel(other.channel)
{
}

ChannelAndDeviceRepObj::ChannelAndDeviceRepObj(ChannelAndDeviceRepObj&& other)
    : id(std::move(other.id))
    , channel(std::move(other.channel))
{
}

ChannelAndDeviceRepObj& ChannelAndDeviceRepObj::operator=(const ChannelAndDeviceRepObj& other)
{
    id = other.id;
    channel = other.channel;
    return *this;
}

ChannelAndDeviceRepObj& ChannelAndDeviceRepObj::operator=(ChannelAndDeviceRepObj&& other)
{
    id = std::move(other.id);
    channel = std::move(other.channel);
    return *this;
}
}
}
