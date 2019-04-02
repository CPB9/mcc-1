#pragma once
#include "mcc/Config.h"
#include <string>
#include "mcc/msg/ptr/Message.h"
#include "mcc/msg/Fwd.h"
#include "mcc/msg/Stats.h"
#include "mcc/msg/obj/Device.h"
#include "mcc/msg/ptr/Defs.h"

namespace mccmsg {
namespace device{

class MCC_MSG_DECLSPEC ConnectObj
{
public:
    ConnectObj();
    ConnectObj(const Device& device, const Channel& channel, bool isConnect);
    ~ConnectObj();

    ConnectObj(const ConnectObj& other);
    ConnectObj(ConnectObj&& other);
    ConnectObj& operator=(const ConnectObj& other);
    ConnectObj& operator=(ConnectObj&& other);

    bool    _isConnect;
    Device  _device;
    Channel _channel;
};

class Connect_Response_Tag {};

class MCC_MSG_DECLSPEC ConnectedObj
{
public:
    ConnectedObj(bool isConnected, const Device& device, Channel channel);
    ~ConnectedObj();
    bool isConnected() const;
    const Device& device() const;
    const Channel& channel() const;

    ConnectedObj(const ConnectedObj& other);
    ConnectedObj(ConnectedObj&& other);
    ConnectedObj& operator=(const ConnectedObj& other);
    ConnectedObj& operator=(ConnectedObj&& other);

private:
    bool _isConnected;
    Device _device;
    Channel _channel;
};

}

MSG_STANDART_SET(device);
MSG_DYNAMIC_SET(device);
MSG_DECLARE_REQ(device, Connect_Request, ConnectObj, Connect_Response, Connect_Response_Tag);
MSG_DECLARE_NOT(device, State, StatDevice);
MSG_DECLARE_NOT(device, Connected, device::ConnectedObj);

}
