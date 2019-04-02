#pragma once
#include <cstdint>
#include <vector>
#include <caf/actor.hpp>
#include <bmcl/MemWriter.h>
#include "mcc/msg/Fwd.h"
#include "mcc/msg/Packet.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4244)
#endif
#define MAVLINK_USE_MESSAGE_INFO
#define MAVLINK_EXTERNAL_RX_STATUS

#include <mavlink/mavlink_types.h>
extern mavlink_status_t m_mavlink_status[MAVLINK_COMM_NUM_BUFFERS];
#include "mavlink/standard/mavlink.h"
#include "mavlink/mavlink_get_info.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace caf { class event_based_actor; }

namespace mccmav {

const char* ackTypeToString(uint8_t type);
const char* frameToString(MAV_FRAME frame);
const char* modeToString(MAV_MODE mode);
const char* resultToString(MAV_RESULT result);

mccmsg::NetVariant toNetVariant(const mavlink_param_union_t& paramUnion);

struct MavlinkMessageRc : public mavlink_message_t, public mcc::RefCountable
{
    MavlinkMessageRc();
    MavlinkMessageRc(const mavlink_message_t& m);
    ~MavlinkMessageRc() override;
};
using MavlinkMessagePtr = bmcl::Rc<const MavlinkMessageRc>;

}