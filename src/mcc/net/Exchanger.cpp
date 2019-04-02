#include "mcc/msg/Stats.h"
#include "mcc/net/Exchanger.h"
#include "mcc/net/NetLoggerInf.h"
#include <bmcl/Utils.h>
#include <bmcl/MakeRc.h>
#include <caf/actor.hpp>
#include <caf/send.hpp>
#include <caf/allowed_unsafe_message_type.hpp>

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::PacketPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(bmcl::SystemTime);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Channel);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::StatChannel);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccnet::LogWriteCreator);

namespace mccnet {

struct DefaultExchanger::DefaultExchangerImpl
{
    DefaultExchangerImpl(const mccmsg::Channel& channel, const caf::actor& broker, const caf::actor& logger, const LogWriteCreator& creator)
        : channel(channel), broker(broker), logger(channel, broker, logger, creator) {}
    mccmsg::Channel channel;
    caf::actor broker;
    LogSender logger;
};

DefaultExchanger::DefaultExchanger(const mccmsg::Channel& channel, const caf::actor& broker, const caf::actor& logger, const LogWriteCreator& creator)
    : _impl(std::make_unique<DefaultExchangerImpl>(channel, broker, logger, creator))
{
}

DefaultExchanger::~DefaultExchanger()
{
}

void DefaultExchanger::changeLog(bool state, bool isConnected)
{
    _impl->logger.changeLog(state, isConnected);
}

void DefaultExchanger::onRcv(const void * start, std::size_t size)
{
    const uint8_t* s = (const uint8_t*)start;
    auto p = bmcl::makeRc<mccmsg::Packet>(s, size);
    _impl->logger.onRcv(p);
    caf::anon_send(_impl->broker, atom_rcvd::value, _impl->channel, std::move(p));
}

void DefaultExchanger::onRcvBad(const void * start, std::size_t size)
{
    _impl->logger.onRcvBad(start, size);
}

void DefaultExchanger::onSent(std::size_t req_id, mccmsg::PacketPtr&& pkt, caf::error&& err)
{
    _impl->logger.onSent(req_id, pkt, err);
    if (err)
        caf::anon_send(_impl->broker, atom_sent::value, _impl->channel, req_id, std::move(err));
}

void DefaultExchanger::onStats(const mccmsg::StatChannel& stats)
{
    caf::anon_send(_impl->broker, _impl->channel, stats);
}

void DefaultExchanger::onConnected()
{
    caf::anon_send(_impl->broker, activated_atom::value, _impl->channel);
    _impl->logger.onConnected();
}

void DefaultExchanger::onDisconnected(const caf::error& err)
{
    caf::anon_send(_impl->broker, deactivated_atom::value, err, _impl->channel);
    _impl->logger.onDisconnected(err);
}

const mccmsg::Channel& DefaultExchanger::channel() const
{
    return _impl->channel;
}

const caf::actor& DefaultExchanger::broker() const
{
    return _impl->broker;
}

}
