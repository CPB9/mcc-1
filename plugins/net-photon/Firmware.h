#pragma once
#include "mcc/Config.h"
#include <string>
#include <memory>
#include <bmcl/Buffer.h>
#include <photon/groundcontrol/Packet.h>
#include <photon/groundcontrol/ProjectUpdate.h>
#include <photon/groundcontrol/NumberedSub.h>
#include "mcc/msg/Objects.h"
#include "mcc/msg/TmView.h"
#include "mcc/msg/ptr/Firmware.h"
#include "mcc/msg/ptr/Firmware.h"

namespace photon { class NodeView; }
namespace photon { class NodeViewUpdater; }
namespace photon { struct PacketResponse; }

namespace mccmsg { class TmUavState; };

namespace mccphoton {

class Firmware;
using FirmwarePtr = bmcl::Rc<const Firmware>;

class Firmware : public mccmsg::IFirmware
{
public:
    Firmware(const mccmsg::ProtocolValue& id, const ::photon::ProjectUpdate::ConstPointer&, const mccmsg::PropertyDescriptionPtrs& req, const mccmsg::PropertyDescriptionPtrs& opt);
    ~Firmware() override;
    bmcl::Buffer encode() const override;
    static std::string getName(bmcl::Bytes bytes);
    static std::string getNameFromHash(bmcl::Bytes bytes);
    static bmcl::OptionRc<const Firmware> decode(const mccmsg::ProtocolValue& id, bmcl::Bytes bytes);
    const ::photon::ProjectUpdate::ConstPointer& pkg() const { return _pkg; }

private:
    ::photon::ProjectUpdate::ConstPointer _pkg;
};

class TmView : public mccmsg::ITmView
{
public:
    TmView(const mccmsg::Device& device, const FirmwarePtr&, const bmcl::Rc<photon::NodeView>& statuses, const bmcl::Rc<photon::NodeView>& events, const bmcl::Rc<photon::NodeView>& stats);
    ~TmView() override;
    const FirmwarePtr& firmware() const;
    const bmcl::Rc<::photon::NodeView>& statusView() const;
    const bmcl::Rc<::photon::NodeView>& eventView() const;
    const bmcl::Rc<::photon::NodeView>& statsView() const;
private:
    FirmwarePtr _firmware;
    bmcl::Rc<::photon::NodeView> _statusView;
    bmcl::Rc<::photon::NodeView> _eventView;
    bmcl::Rc<::photon::NodeView> _statsView;
};

class ITmUpdateVisitor;

class TmViewUpdatePhoton : public mccmsg::ITmViewUpdate
{
public:
    TmViewUpdatePhoton(const mccmsg::Device& device);
    ~TmViewUpdatePhoton() override;
    virtual void visit(ITmUpdateVisitor&) const = 0;
private:
};

class TmUpdateSES : public TmViewUpdatePhoton
{
public:
    TmUpdateSES(const mccmsg::Device& device, const bmcl::Rc<photon::NodeViewUpdater>& statusUpdate, const bmcl::Rc<photon::NodeViewUpdater>& eventUpdate, const bmcl::Rc<photon::NodeViewUpdater>& statsUpdate);
    ~TmUpdateSES() override;
    void visit(ITmUpdateVisitor&) const override;
    const bmcl::Rc<photon::NodeViewUpdater>& statusUpdate() const;
    const bmcl::Rc<photon::NodeViewUpdater>& eventUpdate() const;
    const bmcl::Rc<photon::NodeViewUpdater>& statsUpdate() const;
private:
    bmcl::Rc<photon::NodeViewUpdater> _statusUpdate;
    bmcl::Rc<photon::NodeViewUpdater> _eventUpdate;
    bmcl::Rc<photon::NodeViewUpdater> _statsUpdate;
};

class TmUpdateSub : public TmViewUpdatePhoton
{
public:
    TmUpdateSub(const mccmsg::Device& device, ::photon::NumberedSub sub, const bmcl::SharedBytes& value);
    ~TmUpdateSub() override;
    void visit(ITmUpdateVisitor&) const override;
    ::photon::NumberedSub sub() const;
    const bmcl::SharedBytes& value() const;
private:
    ::photon::NumberedSub _sub;
    bmcl::SharedBytes _value;
};

class ITmUpdateVisitor
{
public:
    ITmUpdateVisitor();
    virtual ~ITmUpdateVisitor();
    virtual void visit(const TmUpdateSES*) = 0;
    virtual void visit(const TmUpdateSub*) = 0;
};

class TmStorage : public mccmsg::ITmStorage, public ITmUpdateVisitor
{
public:
    explicit TmStorage(const bmcl::OptionRc<const TmView>&);
    ~TmStorage() override;

    void set(const mccmsg::ITmView*) override;
    void update(const mccmsg::ITmViewUpdate*) override;

private:
    void visit(const TmUpdateSES*) override;
    void visit(const TmUpdateSub*) override;
    FirmwarePtr _firmware;

    bmcl::OptionRc<mccmsg::TmUavState> _uavstate;
};

class TmPacketResponsePhoton : public mccmsg::ITmPacketResponse
{
public:
    TmPacketResponsePhoton(const mccmsg::Device& device, const photon::PacketResponse&);
    ~TmPacketResponsePhoton() override;
    const photon::PacketResponse& response() const;
private:
    photon::PacketResponse _response;
};

class CmdPacketPhoton : public mccmsg::CmdPacketRequest
{
public:
    CmdPacketPhoton(const mccmsg::Device& device, const photon::PacketRequest& packet, bool hasResponse);
    CmdPacketPhoton(const mccmsg::Group& group, const mccmsg::Device& device, const photon::PacketRequest& packet, bool hasResponse);
    ~CmdPacketPhoton();
    const char* nameXXX() const override;
    const char* info() const override;
    const photon::PacketRequest& packet() const;
    bool hasResponse() const;
private:
    bool _hasResponse;
    std::unique_ptr<photon::PacketRequest> _packet;
};
}
