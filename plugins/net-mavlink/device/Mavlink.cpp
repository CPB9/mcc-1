#include <vector>
#include <sstream>

#include <fmt/format.h>

#include <bmcl/Buffer.h>
#include <bmcl/ArrayView.h>
#include <bmcl/MemWriter.h>
#include <bmcl/Logging.h>


#include "../device/Mavlink.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4244)
#endif
#include "mavlink/standard/mavlink.h"
#include "mavlink/mavlink_helpers.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

mavlink_status_t m_mavlink_status[MAVLINK_COMM_NUM_BUFFERS];

namespace mccmav {

DeviceId MavlinkCoder::device_id(const mccmsg::Packet& pkt)
{
    if (pkt[0] == MAVLINK_STX_MAVLINK1)
        return pkt[3];
    else if (pkt[0] == MAVLINK_STX)
        return pkt[5];

    assert(false);
    return -1;
}

//uint8_t MavlinkCoder::version = 3;
//uint8_t MavlinkCoder::messageLengthByMessageId[] = MAVLINK_MESSAGE_LENGTHS;
//uint8_t MavlinkCoder::crcExtraByMessageId[] = MAVLINK_MESSAGE_CRCS;

mccnet::SearchResult MavlinkCoder::decodePacket(const void* start, std::size_t size)
{
    bmcl::MemReader reader(start, size);
    if (reader.size() < 6)
        return mccnet::SearchResult(0);

    mavlink_message_t bufferMessage = {};
    mavlink_status_t bufferStatus = {};
    mavlink_message_t outMessage = {};
    mavlink_status_t outStatus = {};
    while (reader.sizeLeft() > 0)
    {
        uint8_t res = mavlink_frame_char_buffer(&bufferMessage, &bufferStatus, reader.readUint8(), &outMessage, &outStatus);
        if(bufferStatus.parse_state != MAVLINK_PARSE_STATE_GOT_STX && reader.sizeRead() == 1)
            break;

        if (res != MAVLINK_FRAMING_OK)
            continue;

        uint8_t stx = *(uint8_t*)start;
        if (stx != MAVLINK_STX_MAVLINK1 && stx != MAVLINK_STX)
        {
            assert(false);
        }
        return mccnet::SearchResult(0, reader.sizeRead());
    }

    return mccnet::SearchResult(1);
}

std::string toString(const bmcl::Bytes& bytes)
{
    std::ostringstream str(std::ios_base::ate);
    str << "{" << std::hex << std::showbase;
    for (auto i: bytes)
        str << static_cast<int>(i) << ", ";
    str << "}";
    return str.str();
}
}
