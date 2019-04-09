#pragma once
#include "mcc/Config.h"
#include <vector>
#include "mcc/msg/Fwd.h"
#include "mcc/msg/Objects.h"

namespace mccmsg {

using FormationEntries = std::vector<FormationEntry> ;

using Radars = std::vector<Radar>;
using RadarDescription = bmcl::Rc<const RadarDescriptionObj>;
using RadarDescriptions = std::vector<RadarDescription>;

using Protocols = std::vector<Protocol>;
using ProtocolDescription = bmcl::Rc<const ProtocolDescriptionObj>;
using ProtocolDescriptions = std::vector<ProtocolDescription>;

using Properties = std::vector<Property>;
using PropertyDescriptionPtr = bmcl::Rc<const PropertyDescription>;
using PropertyDescriptionPtrs = std::vector<PropertyDescriptionPtr>;

using Channels = std::vector<Channel>;
using ChannelDescription = bmcl::Rc<const ChannelDescriptionObj>;
using ChannelDescriptions = std::vector<ChannelDescription>;

using DeviceIds = std::vector<DeviceId>;
using Devices = std::vector<Device>;
using DeviceDescription = bmcl::Rc<const DeviceDescriptionObj>;
using DeviceDescriptions = std::vector<DeviceDescription>;

using Groups = std::vector<Group>;

using DeviceUis = std::vector<DeviceUi>;
using DeviceUiDescription = bmcl::Rc<const DeviceUiDescriptionObj>;
using DeviceUiDescriptions = std::vector<DeviceUiDescription>;

using Firmwares = std::vector<Firmware>;
using FirmwareDescription = bmcl::Rc<const FirmwareDescriptionObj>;
using FirmwareDescriptions = std::vector<FirmwareDescription>;

using TmSessions = std::vector<TmSession>;
using TmSessionDescription = bmcl::Rc<const TmSessionDescriptionObj>;
using TmSessionDescriptions = std::vector<TmSessionDescription>;

using ProtocolIds = std::vector<ProtocolId>;
using StatDevices = std::vector<StatDevice>;
using StatChannels = std::vector<StatChannel>;
using CmdParams = std::vector<CmdParam>;

using TmAnyPtr = bmcl::Rc<const TmAny>;
using TmRoutePtr = bmcl::Rc<const TmRoute>;
using TmRoutesListPtr = bmcl::Rc<const TmRoutesList>;
using TmCommonCalibrationStatusPtr = bmcl::Rc<const TmCommonCalibrationStatus>;
using TmCalibrationPtr = bmcl::Rc<const TmCalibration>;
using TmGroupStatePtr = bmcl::Rc<const TmGroupState>;

using DevReqPtr = bmcl::Rc<const DevReq>;
using CmdParamListPtr = bmcl::Rc<const CmdParamList>;
using CmdRouteSetPtr = bmcl::Rc<const CmdRouteSet>;
using CmdRouteSetNoActivePtr = bmcl::Rc<const CmdRouteSetNoActive>;
using CmdRouteSetActivePtr = bmcl::Rc<const CmdRouteSetActive>;
using CmdRouteSetActivePointPtr = bmcl::Rc<const CmdRouteSetActivePoint>;
using CmdRouteSetDirectionPtr = bmcl::Rc<const CmdRouteSetDirection>;
using CmdRouteGetPtr = bmcl::Rc<const CmdRouteGet>;
using CmdRouteGetListPtr = bmcl::Rc<const CmdRouteGetList>;
using CmdRouteCreatePtr = bmcl::Rc<const CmdRouteCreate>;
using CmdRouteRemovePtr = bmcl::Rc<const CmdRouteRemove>;
using CmdRouteClearPtr = bmcl::Rc<const CmdRouteClear>;
using CmdFileGetListPtr = bmcl::Rc<const CmdFileGetList>;
using CmdFileUploadPtr = bmcl::Rc<const CmdFileUpload>;
using CmdFileDownloadPtr = bmcl::Rc<const CmdFileDownload>;
using CmdGetFrmPtr = bmcl::Rc<const CmdGetFrm>;
using CmdGetTmViewPtr = bmcl::Rc<const CmdGetTmView>;
using CmdCalibrationStartPtr = bmcl::Rc<const CmdCalibrationStart>;
using CmdCalibrationCancelPtr = bmcl::Rc<const CmdCalibrationCancel>;
using CmdParamReadPtr = bmcl::Rc<const CmdParamRead>;
using CmdParamWritePtr = bmcl::Rc<const CmdParamWrite>;
using CmdGroupNewPtr = bmcl::Rc<const CmdGroupNew>;
using CmdGroupDeletePtr = bmcl::Rc<const CmdGroupDelete>;
using CmdGroupAttachPtr = bmcl::Rc<const CmdGroupAttach>;
using CmdGroupDetachPtr = bmcl::Rc<const CmdGroupDetach>;
using CmdGroupSwitchPtr = bmcl::Rc<const CmdGroupSwitch>;

using CmdRespAnyPtr = bmcl::Rc<const CmdRespAny>;

}
