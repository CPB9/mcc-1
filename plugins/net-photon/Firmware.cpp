#include <fmt/format.h>
#include <bmcl/MakeRc.h>
#include <bmcl/Result.h>
#include <bmcl/Logging.h>
#include <bmcl/String.h>
#include <bmcl/FixedArrayView.h>
#include <bmcl/Bytes.h>
#include <bmcl/SharedBytes.h>
#include <bmcl/MemReader.h>
#include <decode/core/Diagnostics.h>
#include <decode/parser/Project.h>
#include <photon/model/CoderState.h>
#include <photon/model/ValueInfoCache.h>
#include <photon/model/NodeViewUpdater.h>
#include <photon/groundcontrol/Packet.h>
#include <photon/groundcontrol/ProjectUpdate.h>
#include <photongen/groundcontrol/Validator.hpp>
#include <Photon.hpp>
#include "mcc/msg/exts/UavState.h"

#include "Firmware.h"

namespace mccphoton {

std::string Firmware::getNameFromHash(bmcl::Bytes bytes)
{
    return bmcl::bytesToHexStringLower(bytes);
}

std::string Firmware::getName(bmcl::Bytes bytes)
{
    return getNameFromHash(decode::Project::hash(bytes));
}

Firmware::Firmware(const mccmsg::ProtocolValue& id, const ::photon::ProjectUpdate::ConstPointer& update, const mccmsg::PropertyDescriptionPtrs& req, const mccmsg::PropertyDescriptionPtrs& opt)
    : mccmsg::IFirmware(id, req, opt), _pkg(update)
{
}

Firmware::~Firmware()
{
}

bmcl::Buffer Firmware::encode() const
{
    return _pkg->serialize();
}

bmcl::OptionRc<const Firmware> Firmware::decode(const mccmsg::ProtocolValue& id, bmcl::Bytes bytes)
{
    ::photon::ProjectUpdateResult r = ::photon::ProjectUpdate::fromMemory(bytes);
    if (r.isErr())
    {
        BMCL_WARNING() << "Не удалось десериализовать прошивку " << id.protocol().toStdString() << " " << r.takeErr();
        return bmcl::None;
    }

    assert(getName(bytes) == id.value());
    return bmcl::makeRc<const Firmware>(id, r.take(), mccmsg::PropertyDescriptionPtrs(), mccmsg::PropertyDescriptionPtrs());

}

TmViewUpdatePhoton::TmViewUpdatePhoton(const mccmsg::Device& device) : mccmsg::ITmViewUpdate(device) {}
TmViewUpdatePhoton::~TmViewUpdatePhoton() {}

TmUpdateSES::TmUpdateSES(const mccmsg::Device& device, const bmcl::Rc<photon::NodeViewUpdater>& statusUpdate, const bmcl::Rc<photon::NodeViewUpdater>& eventUpdate, const bmcl::Rc<photon::NodeViewUpdater>& statsUpdate)
    : TmViewUpdatePhoton(device), _statusUpdate(statusUpdate), _eventUpdate(eventUpdate), _statsUpdate(statsUpdate)
{
}

TmUpdateSES::~TmUpdateSES() {}
const bmcl::Rc<photon::NodeViewUpdater>& TmUpdateSES::statusUpdate() const { return _statsUpdate; }
const bmcl::Rc<photon::NodeViewUpdater>& TmUpdateSES::eventUpdate() const { return _eventUpdate; }
const bmcl::Rc<photon::NodeViewUpdater>& TmUpdateSES::statsUpdate() const { return _statsUpdate; }
void TmUpdateSES::visit(ITmUpdateVisitor& visitor) const { visitor.visit(this); }

TmUpdateSub::TmUpdateSub(const mccmsg::Device& device, ::photon::NumberedSub sub, const bmcl::SharedBytes& value) : TmViewUpdatePhoton(device), _sub(sub), _value(value) {}
TmUpdateSub::~TmUpdateSub() {}
::photon::NumberedSub TmUpdateSub::sub() const { return _sub; }
const bmcl::SharedBytes& TmUpdateSub::value() const { return _value; }
void TmUpdateSub::visit(ITmUpdateVisitor& visitor) const { visitor.visit(this); }

ITmUpdateVisitor::ITmUpdateVisitor() {}
ITmUpdateVisitor::~ITmUpdateVisitor() {}

TmView::TmView(const mccmsg::Device& device, const FirmwarePtr& f, const bmcl::Rc<photon::NodeView>& statuses, const bmcl::Rc<photon::NodeView>& events, const bmcl::Rc<photon::NodeView>& stats)
    : mccmsg::ITmView(device), _firmware(f), _statusView(statuses), _eventView(events), _statsView(stats) {}
TmView::~TmView() {}
const FirmwarePtr& TmView::firmware() const { return _firmware; }
const bmcl::Rc<::photon::NodeView>& TmView::statusView() const { return _statusView; }
const bmcl::Rc<::photon::NodeView>& TmView::eventView() const { return _eventView; }
const bmcl::Rc<::photon::NodeView>& TmView::statsView() const { return _statsView; }

TmStorage::TmStorage(const bmcl::OptionRc<const TmView>& v)
    : mccmsg::ITmStorage()
    , _uavstate(new mccmsg::TmUavState(counter()))
{
}
TmStorage::~TmStorage(){}
void TmStorage::set(const mccmsg::ITmView* v)
{
    auto view = static_cast<const TmView*>(v);
    if (_firmware == view->firmware())
        return;
    removeAllHandlers();
    _firmware = view->firmware();
}
void TmStorage::update(const mccmsg::ITmViewUpdate* u)
{
    const TmViewUpdatePhoton* update = static_cast<const TmViewUpdatePhoton*>(u);
    update->visit(*this);
}

void TmStorage::visit(const TmUpdateSES* u)
{
}
void TmStorage::visit(const TmUpdateSub* u)
{
    bmcl::MemReader reader(u->value().view());
    ::photon::CoderState state(::photon::OnboardTime::now());

    auto v = _firmware->pkg()->interface();
    auto sub = u->sub();

    if (sub == v->statusMsgNavAllSub())
    {
//         photongen::nav::statuses::All msg;
//         if (photongenDeserialize(&msg, &reader, &state))
//         {
//             _motion.position = mccgeo::Position(msg.latLon.latitude(), msg.latLon.longitude(), msg.altitude);
//             _motion.velocity = mccgeo::Position(msg.velocity.x(), msg.velocity.y(), msg.velocity.z());
//             _motion.speed = std::hypot(std::hypot(msg.velocity.x(), msg.velocity.y()), msg.velocity.z());
//             _motion.orientation = mccgeo::Attitude(msg.orientation.heading(), msg.orientation.pitch(), msg.orientation.roll());
//             _helper.sendTrait(new mccmsg::TmMotion(_id.device(), _motion));
//         }
    }
    else if (sub == v->statusMsgNavStateSub())
    {
        photongen::nav::statuses::State msg;
        if (photongenDeserialize(&msg, &reader, &state))
        {
//             _armed = msg.deviceState.isArmed();
//             _battery = msg.deviceState.battery();
            _uavstate->set(u->time());
        }
    }
}

TmPacketResponsePhoton::TmPacketResponsePhoton(const mccmsg::Device& device, const photon::PacketResponse& r) : mccmsg::ITmPacketResponse(device), _response(r) {}
TmPacketResponsePhoton::~TmPacketResponsePhoton() {}
const photon::PacketResponse& TmPacketResponsePhoton::response() const { return _response; }


CmdPacketPhoton::CmdPacketPhoton(const mccmsg::Device& device, const photon::PacketRequest& packet, bool hasResponse)
    : mccmsg::CmdPacketRequest(device), _packet(std::make_unique<photon::PacketRequest>(packet)), _hasResponse(hasResponse)
{
}
CmdPacketPhoton::CmdPacketPhoton(const mccmsg::Group& group, const mccmsg::Device& device, const photon::PacketRequest& packet, bool hasResponse)
    : mccmsg::CmdPacketRequest(device), _packet(std::make_unique<photon::PacketRequest>(packet)), _hasResponse(hasResponse)
{
}
CmdPacketPhoton::~CmdPacketPhoton() {}
const photon::PacketRequest& CmdPacketPhoton::packet() const { return *_packet; }
bool CmdPacketPhoton::hasResponse() const { return _hasResponse; }
const char* CmdPacketPhoton::nameXXX() const { return "PacketPhoton"; }
const char* CmdPacketPhoton::info() const { return "Команда-photon"; }

}
