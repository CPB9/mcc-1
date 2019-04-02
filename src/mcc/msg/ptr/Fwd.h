#pragma once
#include "mcc/Config.h"
#include <vector>
#include <mcc/msg/ptr/Defs.h>
#include <mcc/msg/Objects.h>
#include <mcc/msg/FwdExt.h>

namespace mccmsg
{
    using MessagePtr = bmcl::Rc<const Message>;
    using RequestPtr = bmcl::Rc<const Request>;
    using DbRequestPtr = bmcl::Rc<const DbReq>;
    using ResponsePtr = bmcl::Rc<const Response>;
    using NotificationPtr = bmcl::Rc<const Notification>;
    using CancelPtr = bmcl::Rc<const Cancel>;
    using Request_StatePtr = bmcl::Rc<const Request_State>;

    template<typename T> class NoteImpl;
    template<typename T> class ResponseImpl;
    template<typename T, typename Tag, typename R> class RequestImpl;

    class ProtocolValue;
}


namespace mccmsg
{

namespace channel
{
    using ObjName = Channel;
    using ObjNames = Channels;
    using ObjDscr = ChannelDescription;
    using ObjDscrs = ChannelDescriptions;
    using mccmsg::ProtocolValue;
    using StatChannels = std::vector<StatChannel>;
}
MSG_DECLARE_NOT_FWD(channel, State, StatChannel);
MSG_STANDART_SET_FWD(channel);
MSG_DYNAMIC_SET_FWD(channel);


namespace device
{
    class ConnectObj;
    class ConnectedObj;
    class Update_RequestObj;
    class Connect_Response_Tag;
    class Update_Response_Tag;

    using ObjName = Device;
    using ObjNames = Devices;
    using ObjDscr = DeviceDescription;
    using ObjDscrs = DeviceDescriptions;
    using mccmsg::ProtocolValue;
    using StatDevices = std::vector<StatDevice>;
}
MSG_STANDART_SET_FWD(device);
MSG_DYNAMIC_SET_FWD(device);
MSG_DECLARE_REQ_FWD(device, Connect_Request, ConnectObj, Connect_Response, Connect_Response_Tag);
MSG_DECLARE_NOT_FWD(device, State, StatDevice);
MSG_DECLARE_NOT_FWD(device, Connected, device::ConnectedObj);

namespace deviceUi
{
    using ObjName = DeviceUi;
    using ObjNames = DeviceUis;
    using ObjDscr = DeviceUiDescription;
    using ObjDscrs = DeviceUiDescriptions;
    using mccmsg::ProtocolValue;
}
MSG_STANDART_SET_FWD(deviceUi);

namespace protocol
{
    using ObjName = Protocol;
    using ObjNames = Protocols;
    using ObjDscr = ProtocolDescription;
    using ObjDscrs = ProtocolDescriptions;
    using mccmsg::ProtocolValue;
}
MSG_STANDART_SET_FWD(protocol);

namespace firmware
{
    using ObjName = Firmware;
    using ObjNames = Firmwares;
    using ObjDscr = FirmwareDescription;
    using ObjDscrs = FirmwareDescriptions;
    using mccmsg::ProtocolValue;
}
MSG_STANDART_SET_FWD(firmware);

namespace radar
{
    using ObjName = Radar;
    using ObjNames = Radars;
    using ObjDscr = RadarDescription;
    using ObjDscrs = RadarDescriptions;
    using mccmsg::ProtocolValue;
}
MSG_STANDART_SET_FWD(radar);

namespace tmSession
{
    using ObjName = TmSession;
    using ObjNames = TmSessions;
    using ObjDscr = TmSessionDescription;
    using ObjDscrs = TmSessionDescriptions;
    using mccmsg::ProtocolValue;
}
MSG_STANDART_SET_FWD(tmSession);

namespace tm
{
    class LogObj;
    class DumpObj;
    class Dump_Response_Tag;
}
MSG_DECLARE_REQ_FWD(tm, Dump_Request, DumpObj, Dump_Response, Dump_Response_Tag);
MSG_DECLARE_NOT_FWD(tm, Item, bmcl::Rc<const mccmsg::TmAny>);
MSG_DECLARE_NOT_FWD(tm, Log, tm::LogObj);

namespace cmd
{
    class Log;
    class Visitor;
    using Param = CmdParam;
    using Params = CmdParams;
}

namespace advanced
{
    class ChannelAndDeviceReqObj;
    class ChannelAndDeviceRepObj;

}
MSG_DECLARE_REQ_FWD(advanced, ChannelAndDeviceRegister_Request, ChannelAndDeviceReqObj, ChannelAndDeviceRegister_Response, ChannelAndDeviceRepObj);

}
