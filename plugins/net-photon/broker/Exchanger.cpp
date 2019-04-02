#include <bmcl/Bytes.h>
#include <bmcl/Logging.h>
#include <photon/groundcontrol/GroundControl.h>
#include "mcc/msg/Objects.h"
#include "mcc/net/NetLoggerInf.h"
#include "../broker/Exchanger.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::PacketPtr);

namespace mccphoton {

Exchanger::Exchanger(const mccmsg::Channel& channel, const caf::actor& broker, const caf::actor& logger) : DefaultExchanger(channel, broker, logger, mccnet::DefaultLogWriter::creator(channel))
{
}

mccnet::SearchResult Exchanger::find(const void* data, std::size_t size)
{
    auto r = ::photon::GroundControl::findPacket(bmcl::Bytes((const uint8_t*)data, size));
    if (r.dataSize > 0)
        return mccnet::SearchResult(r.junkSize, r.dataSize);
    return mccnet::SearchResult(r.junkSize);
}
}