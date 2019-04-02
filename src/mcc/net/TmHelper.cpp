#include <caf/send.hpp>
#include <caf/actor.hpp>
#include <caf/event_based_actor.hpp>
#include "mcc/net/TmHelper.h"
#include "mcc/msg/TmView.h"
#include "mcc/msg/Stats.h"
#include "mcc/msg/ptr/Tm.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mcc::Rc<mccmsg::ITmViewUpdate>);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Device);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NotificationPtr)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::StatDevice)

namespace mccnet {

TmHelper::TmHelper(const mccmsg::ProtocolId& id, const caf::actor& core, caf::event_based_actor* self)
    : _id(id), _core(core), _self(self)
{
}

TmHelper::~TmHelper() {}
const mccmsg::Device& TmHelper::device() const { return _id.device(); }
const mccmsg::Protocol& TmHelper::protocol() const { return _id.protocol(); }
const mccmsg::ProtocolId& TmHelper::id() const { return _id; }
const caf::actor& TmHelper::core() const { return _core; }

void TmHelper::log(bmcl::LogLevel logLevel, std::string&& str) const
{
    caf::send_as(_self, _core, mccmsg::makeNote(new mccmsg::tm::Log(logLevel, _self->name(), _id.device(), str)));
}

void TmHelper::log_text(bmcl::LogLevel logLevel, bmcl::StringView str) const
{
    caf::send_as(_self, _core, mccmsg::makeNote(new mccmsg::tm::Log(logLevel, _self->name(), _id.device(), str)));
}

void TmHelper::sendTrait(const mccmsg::TmAny* tm) const
{
    caf::send_as(_self, _core, mccmsg::makeTm(tm));
}

void TmHelper::sendStats(const mccmsg::StatDevice& state) const
{
    caf::send_as(_self, _core, mccmsg::makeNote(new mccmsg::device::State(state)));
}

}
