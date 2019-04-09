#include <caf/event_based_actor.hpp>
#include <bmcl/Logging.h>
#include "mcc/net/NetLoggerInf.h"
#include "../broker/Device.h"
#include "../broker/Channel.h"
#include <assert.h>

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::PacketPtr);

namespace mccphoton {

DItem::DItem(mccmsg::ProtocolId id, const caf::actor& a, caf::event_based_actor* self) : _a(a), _id(id), _self(self)
{
}

DItem::~DItem()
{
    BMCL_DEBUG() << "Устройство удалёно из брокера: " << _id.device().toStdString();
}

void DItem::addChannel(const CItemPtr& c)
{
    if (!hasChannels())
        _self->send(_a, mccnet::connected_atom::value);
    cs.emplace(c->name(), c);
}

void DItem::removeChannel(const mccmsg::Channel& c)
{
    cs.erase(c);
    if (!hasChannels())
        _self->send(_a, mccnet::disconnected_atom::value);
}

void DItem::pushRequest(Request&& r)
{
    if (!hasChannels())
        return;

    queue.push_back(std::move(r));
    if (queue.size() > 1)
        return;

    for (const auto& i : cs)
        i.second->sendIfYouCan();
}

Request DItem::popRequest()
{
    assert(hasRequests());
    Request r = std::move(queue.front());
    queue.pop_front();
    return r;
}

void DItem::pull(const mccmsg::PacketPtr& pkt)
{
    _self->send(_a, pkt);
}
}
