#pragma once
#include "mcc/Config.h"
#include <cstddef>

namespace mccmsg {

using RequestId = std::size_t;

class Device;
class Group;
class DeviceUi;
class Protocol;
class Property;
class Channel;
class Radar;
class Firmware;
class TmSession;

class ErrorDscr;

class RadarDescriptionObj;
class ProtocolId;
class ProtocolDescriptionObj;
class PropertyDescription;
class IFirmware;
class ChannelDescriptionObj;
class DeviceDescriptionObj;
class DeviceUiDescriptionObj;
class FirmwareDescriptionObj;
class NetTcpParams;
class NetUdpParams;
class NetComParams;
class Stat;
class StatDevice;
class StatChannel;
struct GpsSat;
class Route;
class RoutesList;
struct RouteProperties;
class FormationEntry;
class Formation;
class TmSessionDescriptionObj;

class NetVariant;
using CmdParam = NetVariant;
class Packet;

class ProtocolController;
}

namespace mccmsg{
class CmdVisitor;
class CmdParamList;
class CmdRouteSet;
class CmdRouteSetNoActive;
class CmdRouteSetActive;
class CmdRouteSetActivePoint;
class CmdRouteSetDirection;
class CmdRouteGet;
class CmdRouteGetList;
class CmdRouteCreate;
class CmdRouteRemove;
class CmdRouteClear;
class CmdFileGetList;
class CmdFileUpload;
class CmdFileDownload;
class CmdGetFrm;
class CmdCalibrationStart;
class CmdCalibrationCancel;
class CmdParamRead;
class CmdParamWrite;
class CmdGroupNew;
class CmdGroupDelete;
class CmdGroupAttach;
class CmdGroupDetach;
class CmdGroupSwitch;
class CmdPacketRequest;
class CmdGetTmView;

class CmdRespVisitor;
class CmdRespAny;
class CmdFileGetListResp;
class CmdRespEmpty;

}

namespace mccmsg{

class TmVisitor;
class TmAny;
class TmMotion;
class TmRoute;
class TmRoutesList;
class TmCalibration;
class TmCommonCalibrationStatus;
class TmGroupState;

class ITmView;
class ITmViewUpdate;
class ITmStorage;
class ITmPacketResponse;

class Message;
class Request;
class Response;
class DevReq;
class DbReq;
class Notification;
class Cancel;
class Request_State;

class ReqVisitor;
class NoteVisitor;

}

namespace mccmsg
{
    class IPropertyValue;
    class PropertyValues;
    class PropertyDescription;
    class PropertyReadOnly;
    class PropertyTarget;
    class PropertyParachute;
    class PropertyTurnBack;
    class PropertyTurnFront;
    class PropertyWaiting;
    class PropertySwitchRoute;
    class PropertyHome;
    class PropertyLanding;
    class PropertyDirection;
    class PropertySpeed;
    class PropertyFormation;
}
