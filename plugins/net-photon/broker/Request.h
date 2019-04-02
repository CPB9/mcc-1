#pragma once
#include <deque>
#include <caf/response_promise.hpp>
#include "mcc/msg/Packet.h"

namespace mccphoton {

struct Request
{
    Request(mccmsg::PacketPtr&& pkt) : pkt(std::move(pkt))
    {
    }
    mccmsg::PacketPtr pkt;
};
using Queue = std::deque<Request>;
}