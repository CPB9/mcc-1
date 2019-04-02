#include "mcc/msg/ptr/Channel.h"
#include "mcc/msg/ptr/Device.h"
#include "mcc/msg/ptr/DeviceUi.h"
#include "mcc/msg/ptr/Firmware.h"
#include "mcc/msg/ptr/Protocol.h"
#include "mcc/msg/ptr/Radar.h"
#include "mcc/msg/ptr/TmSession.h"
#include "mcc/net/db/obj/Object.h"

namespace mccdb {
namespace dbobj {

template class QueryObject<mccmsg::ChannelDescription, mccmsg::Channel>;
template class QueryObject<mccmsg::DeviceDescription, mccmsg::Device>;
template class QueryObject<mccmsg::DeviceUiDescription, mccmsg::DeviceUi>;
template class QueryObject<mccmsg::FirmwareDescription, mccmsg::Firmware>;
template class QueryObject<mccmsg::ProtocolDescription, mccmsg::Protocol>;
template class QueryObject<mccmsg::RadarDescription, mccmsg::Radar>;
template class QueryObject<mccmsg::TmSessionDescription, mccmsg::TmSession>;

}
}
