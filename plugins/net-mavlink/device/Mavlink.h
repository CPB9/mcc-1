#pragma once
#include <vector>
#include <cassert>
#include <set>
#include <bmcl/Result.h>
#include <bmcl/MemReader.h>

#include "mcc/msg/ptr/Protocol.h"
#include "mcc/msg/Packet.h"
#include "mcc/net/Exchanger.h"

namespace bmcl { class MemWriter; }

namespace mccmav {

typedef uint8_t DeviceId;

class MavlinkCoder
{
public:
//    static uint8_t version;
//    static uint8_t messageLengthByMessageId[];
//    static uint8_t crcExtraByMessageId[];

    static DeviceId device_id(const mccmsg::Packet& pkt);

    static mccnet::SearchResult decodePacket(const void* start, std::size_t size);

    static inline mccnet::SearchResult findPacket(const void* start, std::size_t size)
    {
        if (size == 0)
            return mccnet::SearchResult(0);

        bmcl::MemReader reader(start, size);

        while (!reader.isEmpty())
        {
            auto r = decodePacket(reader.current(), reader.sizeLeft());
            if (r._offset > 0) reader.skip(r._offset);
            if (r._packet.isSome())
            {
                if (r._packet.unwrap() <= reader.sizeLeft())
                    return mccnet::SearchResult(reader.sizeRead(), r._packet.unwrap());
                return mccnet::SearchResult(reader.sizeRead());
            }

            if (r._offset == 0)
                return mccnet::SearchResult(reader.sizeRead());
        }
        assert(size == 0);
        return mccnet::SearchResult(size);
    }
};

struct MavlinkSettings
{
    std::uint8_t system;
    std::uint8_t component;
    std::uint8_t channel;

    explicit MavlinkSettings(const mccmsg::ProtocolId& id)
        : system((uint8_t)id.id())
        , component(0)
        , channel(0)
    {
    }
};
}
