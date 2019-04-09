#include <ctime>
#include <bmcl/Logging.h>
#include "mcc/msg/ptr/Channel.h"
#include "mcc/msg/ptr/Device.h"
#include "mcc/msg/ptr/NoteVisitor.h"
#include "mcc/msg/ptr/ReqVisitor.h"
#include "mcc/msg/Packet.h"
#include "mcc/net/Asio.h"
#include "mcc/net/NetLoggerInf.h"
#include "../broker/Broker.h"
#include "../broker/Connections.h"
#include "../device/Device.h"
#include "../device/Mavlink.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Protocol)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::DbReqPtr)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::DevReqPtr)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::CancelPtr)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NotificationPtr)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::device::Activate_RequestPtr)

namespace mccmav {

const char* Broker::name() const
{
    return _name.c_str();
}

Broker::~Broker()
{
}

std::string Broker::getDeviceName(const mccmsg::ProtocolId& id)
{
    return fmt::format("net.{}.{}", _dscr->info(), id.id());
}

Broker::Broker(caf::actor_config& cfg, const caf::actor& core, const caf::actor& logger, const caf::actor& group, const mccmsg::ProtocolDescription& dscr)
    : caf::event_based_actor(cfg), _core(core), _dscr(dscr)
{
    _name = fmt::format("net.{}.broker", _dscr->info());
    _conns = std::make_unique<Connections>(this, core, logger, group);

    auto grp = system().groups().get_local("notes");
    join(grp);
}

class NoteVisitorX : public mccmsg::NoteVisitor
{
private:
    Broker* _self;
public:
    explicit NoteVisitorX(Broker* self) : _self(self) {}

    using mccmsg::NoteVisitor::visit;

    void visit(const mccmsg::channel::Registered* msg) override
    {
        if (msg->data()._state)
            _self->reqChannel(msg->data()._name);
        else
            _self->_conns->removeChannel(msg->data()._name);
    }
    void visit(const mccmsg::channel::Updated* msg) override
    {
        _self->reqChannel(msg->data()->name());
    }
    void visit(const mccmsg::device::Registered* msg) override
    {
        if (msg->data()._state)
            return;
        _self->_conns->removeDevice(msg->data()._name, mccmsg::make_error(mccmsg::Error::DeviceUnregistered));
    }
    void visit(const mccmsg::device::Connected* msg) override
    {
        if (!msg->data().isConnected())
        {
            _self->_conns->disconnect(msg->data().channel(), msg->data().device());
            return;
        }
        _self->reqChannel(msg->data().channel());
    }
};

class ReqVisitorX : public mccmsg::ReqVisitor
{
private:
    Broker* _self;
public:
    ReqVisitorX(Broker* self)
        : mccmsg::ReqVisitor([self](const mccmsg::Request*) { self->response(caf::sec::unexpected_message); }), _self(self)
    {
    }

    using mccmsg::ReqVisitor::visit;

    void visit(const mccmsg::channel::Activate_Request* msg) override
    {
        _self->_conns->channelActivate(mccmsg::channel::Activate_RequestPtr(msg), _self->make_response_promise());
    }
    void visit(const mccmsg::device::Activate_Request* msg) override
    {
        auto i = _self->_conns->getDevice(msg->data()._name);
        if (i.isNone())
            _self->response(caf::sec::request_receiver_down);
        else
            _self->delegate(i.unwrap(), mccmsg::device::Activate_RequestPtr(msg));
    }
};

void Broker::on_exit()
{
    _conns.reset();
    destroy(_core);
}

caf::behavior Broker::make_behavior()
{
    set_down_handler ( [this](caf::down_msg& dm)
    {
        _conns->deviceDown(dm);
        if (_exitReason.isSome() && !_conns->hasDevices())
        {
            quit();
            return;
        }
    });

    set_exit_handler( [this](caf::exit_msg& em)
    {
        _exitReason = em.reason;
        if (!_conns->hasDevices())
        {
            quit();
            return;
        }

        _conns->removeAllDevices(em.reason);
    });

    reqChannels();

    return
    {
        [this](const mccmsg::NotificationPtr& msg)
        {
            NoteVisitorX visitor(this);
            msg->visit(visitor);
        }
      , [this](const mccmsg::DbRequestPtr& msg)
        {
            ReqVisitorX visitor(this);
            msg->visit(visitor);
            return caf::delegated<mccmsg::ResponsePtr>();
        }
      , [this](const mccmsg::DevReqPtr& msg)
        {
            assert(msg->device().isSome());
            auto i = _conns->getDevice(msg->device().unwrap());
            if (i.isNone())
                response(caf::sec::request_receiver_down);
            else
                delegate(i.unwrap(), msg);
        }
      , [this](const mccmsg::CancelPtr& msg)
        {
            const auto& r = msg->request();
            if (r->kind() != mccmsg::ReqKind::Dev)
                return;
            auto req = bmcl::static_pointer_cast<const mccmsg::DevReq>(r);
            assert(req->device().isSome());
            auto i = _conns->getDevice(req->device().unwrap());
            if (i.isNone())
                return;
            delegate(i.unwrap(), msg);
        }
      , [this](mccnet::req_atom, const mccmsg::Device& dev, mccmsg::PacketPtr& pkt)
        {
            _conns->push(dev, Request(std::move(pkt)));
        }
      , [this](mccnet::atom_rcvd, const mccmsg::Channel& channel, mccmsg::PacketPtr& pkt)
        {
            _conns->pull(channel, std::move(pkt));
        }
      , [this](mccnet::atom_timeout, const mccmsg::Channel& channel)
        {
            _conns->timeout(channel);
        }
      , [this](const mccmsg::Channel& c, const mccmsg::StatChannel& stats)
        {
            auto s = _conns->stats(c, stats);
            send(_core, mccmsg::makeNote(new mccmsg::channel::State(s)));
        }
      , [this](mccnet::activated_atom, const mccmsg::Channel& channel)
        {
            _conns->channelActivated(channel);
            auto s = _conns->getStats(channel);
            if (s.isSome())
                send(_core, mccmsg::makeNote(new mccmsg::channel::State(*s)));
        }
      , [this](mccnet::deactivated_atom, const caf::error& err, const mccmsg::Channel& channel)
        {
            _conns->channelDeactivated(channel, err);
            auto s = _conns->getStats(channel);
            if (s.isSome())
                send(_core, mccmsg::makeNote(new mccmsg::channel::State(*s)));
        }
      , [this](mccnet::activated_atom, const mccmsg::Device& device, bool state)
        {
            _conns->deviceActivated(device, state);
            std::vector<std::size_t> ds;
            for(const auto& i: _conns->devices())
            {
                if (i.second->isActive())
                    ds.push_back(i.second->id().id());
            }

            for (const auto& i : _conns->devices())
            {
                send(i.second->actor(), mccnet::activated_list_atom::value, ds);
            }
        }

    };
}

void Broker::reqChannel(const mccmsg::Channel& name)
{
    request(_core, caf::infinite, mccmsg::makeReq(new mccmsg::channel::Description_Request(name))).then
    (
        [this](const mccmsg::channel::Description_ResponsePtr& rep)
        {
            if (rep->data()->protocol() == _dscr->name())
                _conns->addChannel(rep->data());
            else
                _conns->removeChannel(rep->data()->name());
        }
      , [this](const caf::error&)
        {
        }
    );
}

void Broker::reqChannels()
{
    request(_core, caf::infinite, mccmsg::makeReq(new mccmsg::channel::DescriptionList_Request())).then
    (
        [this](const mccmsg::channel::DescriptionList_ResponsePtr& rep)
        {
            auto tmp = rep->data();
            auto protocol = _dscr->name();
            tmp.erase(std::remove_if(tmp.begin(), tmp.end(), [&](const mccmsg::ChannelDescription& d) { return d->protocol() != protocol; }), tmp.end());
            _conns->sync(tmp);
        }
      , [this](const caf::error&)
        {
        }
    );
}
}
