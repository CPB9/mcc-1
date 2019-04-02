#pragma once
#include "mcc/msg/ptr/Fwd.h"
#include "mcc/msg/ptr/Protocol.h"
#include "mcc/msg/ptr/Tm.h"
#include "mcc/msg/Packet.h"

#include "../broker/Request.h"

namespace mccmav {

class CItem;
using CItemPtr = mcc::Rc<CItem>;
using CItems = std::map<mccmsg::Channel, CItemPtr>;

class DItem : public mcc::RefCountable
{
public:
    DItem(mccmsg::ProtocolId id, const caf::actor& a, caf::event_based_actor* self);
    ~DItem();
    inline const mccmsg::ProtocolId& id() const { return _id; }
    inline const mccmsg::Device& name() const { return _id.device(); }
    inline caf::actor& actor() { return _a; }
    inline const caf::actor& actor() const { return _a; }
    inline bool isSame(const caf::actor_addr& a) const { return _a == a; }
    inline bool isSameId(std::size_t id) const { return _id.id() == id; }
    inline bool hasChannels() const { return !cs.empty(); }
    inline bool hasRequests() const { return !queue.empty(); }
    inline bool isActive() const { return _isActive; }

    void activated(bool isActive);
    void addChannel(const CItemPtr& c);
    void removeChannel(const mccmsg::Channel& c);
    void pushRequest(Request&& r);
    Request popRequest();
    void pull(const mccmsg::PacketPtr& pkt);
private:
    bool _isActive;
    Queue queue;
    CItems cs;
    caf::actor _a;
    mccmsg::ProtocolId _id;
    caf::event_based_actor* _self;
};
using DItemPtr = mcc::Rc<DItem>;
using DItems = std::map<mccmsg::Device, DItemPtr>;
}
