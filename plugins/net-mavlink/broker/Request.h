#pragma once
#include <deque>
#include "mcc/msg/Packet.h"

namespace mccmav {

struct Request
{
    Request(mccmsg::PacketPtr&& pkt)
        : pkt(std::move(pkt))
    {
    }
    mccmsg::PacketPtr pkt;
};
using Queue = std::deque<Request>;
}