#pragma once
#include "mcc/Config.h"
#include "mcc/msg/ptr/Device.h"
#include "mcc/msg/ptr/Channel.h"

namespace mccmsg {
namespace advanced {

class MCC_MSG_DECLSPEC ChannelAndDeviceReqObj
{
public:
    DeviceDescription  device;
    ChannelDescription channel;

    ChannelAndDeviceReqObj();
    ChannelAndDeviceReqObj(const DeviceDescription& d, const ChannelDescription& c);
    ChannelAndDeviceReqObj(const ChannelAndDeviceReqObj& other);
    ChannelAndDeviceReqObj(ChannelAndDeviceReqObj&& other);
    ~ChannelAndDeviceReqObj();

    ChannelAndDeviceReqObj& operator=(const ChannelAndDeviceReqObj& other);
    ChannelAndDeviceReqObj& operator=(ChannelAndDeviceReqObj&& other);
};

class MCC_MSG_DECLSPEC ChannelAndDeviceRepObj
{
public:
    ProtocolId  id;
    Channel     channel;

    ChannelAndDeviceRepObj();
    ChannelAndDeviceRepObj(const ProtocolId& id, const Channel& channel);
    ChannelAndDeviceRepObj(const ChannelAndDeviceRepObj& other);
    ChannelAndDeviceRepObj(ChannelAndDeviceRepObj&& other);
    ~ChannelAndDeviceRepObj();

    ChannelAndDeviceRepObj& operator=(const ChannelAndDeviceRepObj& other);
    ChannelAndDeviceRepObj& operator=(ChannelAndDeviceRepObj&& other);
};
}

MSG_DECLARE_REQ(advanced, ChannelAndDeviceRegister_Request, ChannelAndDeviceReqObj, ChannelAndDeviceRegister_Response, ChannelAndDeviceRepObj);

}
