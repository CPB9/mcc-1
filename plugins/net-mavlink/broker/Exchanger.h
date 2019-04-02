#pragma once
#include <caf/send.hpp>
#include "mcc/msg/Packet.h"
#include "mcc/net/NetLoggerInf.h"

#include "../broker/LogWriter.h"
#include "../device/Mavlink.h"

namespace mccmav {

class Exchanger : public mccnet::DefaultExchanger
{
public:
    Exchanger(const mccmsg::Channel& channel, const caf::actor& broker, const caf::actor& logger)
        : DefaultExchanger(channel, broker, logger, []() -> mccnet::ILogWriterPtr { return std::make_unique<MavLogWriter>("mav"); }) { }
    mccnet::SearchResult find(const void * start, std::size_t size) override { return MavlinkCoder::findPacket(start, size); }
};

}
