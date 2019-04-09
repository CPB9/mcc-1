#include "mcc/msg/FwdExt.h"
#include "mcc/msg/Route.h"
#include "mcc/msg/NetVariant.h"

#include "mcc/msg/Stats.h"
#include "mcc/msg/ptr/Device.h"
#include "mcc/msg/ptr/Firmware.h"
#include "mcc/msg/ptr/Tm.h"
#include "mcc/msg/TmView.h"
#include "mcc/msg/exts/Gps.h"

#include "mcc/msg/Packet.h"
#include "mcc/net/NetLoggerInf.h"
#include "mcc/net/Group.h"
#include "../device/Device.h"
#include "../device/FileReader.h"
#include "../Firmware.h"

#include "../traits/Trait.h"
#include "../traits/TraitRadioCalibration.h"

#include <fmt/format.h>
#include <bmcl/Logging.h>
#include <bmcl/SharedBytes.h>
#include <bmcl/String.h>
#include <bmcl/FixedArrayView.h>
#include <bmcl/MakeRc.h>

#include <decode/ast/Ast.h>
#include <decode/ast/Component.h>
#include <decode/ast/Function.h>
#include <decode/core/Diagnostics.h>
#include <decode/core/StringBuilder.h>
#include <decode/parser/Package.h>
#include <decode/parser/Project.h>

#include <photon/core/Rc.h>
#include <photon/groundcontrol/Atoms.h>
#include <photon/groundcontrol/ProjectUpdate.h>
#include <photon/groundcontrol/TmParamUpdate.h>
#include <photon/groundcontrol/AllowUnsafeMessageType.h>
#include <photon/groundcontrol/GcStructs.h>
#include <photon/groundcontrol/GcCmd.h>
#include <photon/groundcontrol/TmState.h>
#include <photon/groundcontrol/Packet.h>
#include <photon/model/NodeViewUpdater.h>
#include <photon/model/CoderState.h>
#include <photon/model/CmdNode.h>
#include <Photon.hpp>

#include <caf/unit.hpp>
#include <caf/make_message.hpp>
#include <caf/result.hpp>
#include <caf/others.hpp>
#include <caf/actor_ostream.hpp>

DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(bmcl::Rc<const decode::Project>);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(bmcl::Rc<const decode::Device>);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(bmcl::Rc<::photon::NodeView>);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(bmcl::Rc<::photon::NodeViewUpdater>);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(bmcl::SharedBytes);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(::photon::PacketRequest);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(::photon::PacketResponse);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(::photon::Route);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(::photon::GcCmd);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(::photon::NumberedSub);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(::photon::ProjectUpdate::ConstPointer);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(std::vector<::photon::Value>);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(mccnet::GroupState);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(mccnet::GroupDeviceState);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::PacketPtr);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Device);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::ProtocolId);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(bmcl::Rc<mccmsg::ITmViewUpdate>);
DECODE_ALLOW_UNSAFE_MESSAGE_TYPE(bmcl::Rc<mccmsg::ITmView>);

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(photongen::fcu::RcChannels);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::device::Activate_ResponsePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NotificationPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::DbReqPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::CmdCalibrationStartPtr);

namespace mccphoton {

using timerAtom = caf::atom_constant<caf::atom("timer")>;
using cancel_atom = caf::atom_constant<caf::atom("cancel")>;


class Visitor : public mccmsg::CmdVisitor
{
public:
    Visitor(Device* self, const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd)
        : mccmsg::CmdVisitor([&](const mccmsg::DevReq*) { _cmd->sendFailed(mccmsg::Error::CmdUnknown); })
        , _self(self)
        , _cmd(std::move(cmd))
        , _group(group)
    {
    }

    using mccmsg::CmdVisitor::visit;

    void visit(const mccmsg::CmdParamList* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdCalibrationStart* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdCalibrationCancel* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdRouteSet* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdRouteSetNoActive* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdRouteSetActive* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdRouteSetActivePoint* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdRouteSetDirection* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdRouteGet* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdRouteGetList* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdRouteCreate* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdRouteRemove* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdRouteClear* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdFileGetList* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdFileUpload* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdFileDownload* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdParamRead* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdParamWrite* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
    void visit(const mccmsg::CmdPacketRequest* msg) override { _self->execute(_group, std::move(_cmd), *msg); }
private:
    Device* _self;
    mccnet::CmdPtr  _cmd;
    bmcl::Option<mccmsg::Group> _group;
};

static void publishTmEventUpdates(TmHelper* helper, const std::vector<::photon::NodeViewUpdate>& updates)
{
    for (auto& e : updates)
    {
        auto rootNode = (::photon::Node*)e.id();

        for (int i = 0; i < rootNode->numChildren(); ++i)
        {
            auto node = rootNode->childAt(i);
            if (node.isNone())
                continue;

            decode::StringBuilder builder;
            builder.append(node->fieldName());
            builder.appendSpace();
            node->stringify(&builder);
            helper->log_text(bmcl::LogLevel::Info, builder.view());
        }
    }
}



Device::Device(caf::actor_config& cfg, const caf::actor& core, const caf::actor& broker, const caf::actor& group,
               const mccmsg::ProtocolId& id, const std::string& name)
    : caf::event_based_actor(cfg)
    , _core(core)
    , _broker(broker)
    , _group(group)
    , _gc(spawn<::photon::GroundControl, caf::linked>(0, id.id(), this, this))
    , _name(name)
    , _id(id)
    , _stats(id.device())
    , _helper(id, core, this)
    , _isConnected(false)
{
    BMCL_DEBUG() << (int)caf::actor(this).id() << _name.c_str();
    //send(_gc, ::photon::EnableLoggindAtom::value, true);
    send(_group, mccnet::group_dev::value, id, caf::actor_cast<caf::actor>(this));
    _radioCalibration = spawn<TraitRadioCalibration, caf::linked>(_core, this, _id, _name + ".radiocal");
}

const char* Device::name() const
{
    return _name.c_str();
}

void Device::on_exit()
{
    destroy(_radioCalibration);
    destroy(_broker);
    destroy(_core);
    destroy(_gc);
    destroy(_group);
}

void Device::state_changed()
{
    bool state = _stats._isActive && _isConnected;
    if (state)
    {
        send(_gc, ::photon::StartAtom::value);
        send(_gc, ::photon::UpdateFirmware::value);
        request(_gc, caf::infinite, ::photon::SendGcCommandAtom::value, ::photon::GcCmd(::photon::DownloadRouteInfoGcCmd())).then
        (
            [](caf::unit_t) { ; }
          , [](caf::error& e) { ; }
        );
        send(_radioCalibration, activated_atom::value);
    }
    else
    {
        send(_gc, ::photon::StopAtom::value);
        send(_radioCalibration, deactivated_atom::value);
    }
}

void Device::restore_firmware(const mccmsg::ProtocolValue& id)
{
    request(_core, caf::infinite, mccmsg::makeReq(new mccmsg::firmware::DescriptionS_Request(id))).then
    (
        [this](const mccmsg::firmware::Description_ResponsePtr& rep)
        {
            set_firmware(rep->data()->frm());
        }
      , [this](caf::error& err)
        {
            BMCL_WARNING() << "Запрос прошивки из БД завршился с ошибкой! " << system().render(err);
        }
    );
}

void Device::subscribeForTm()
{
    auto v = _pkg->pkg()->interface();
    auto sub = [&](const bmcl::Option<::photon::NumberedSub>& id, const char* name)
    {
        if (id.isNone())
        {
            _helper.log(bmcl::LogLevel::Warning, fmt::format("Тм-кадр в прошивке не найден! {}", name));
            return;
        }

        request(_gc, caf::infinite, ::photon::SubscribeNumberedTmAtom::value, id.unwrap(), caf::actor_cast<caf::actor>(this)).then
        (
            [this, name](bool result)
            {
                if (result)
                    _helper.log(bmcl::LogLevel::Debug, fmt::format("Подписались на тм-кадр {}", name));
                else
                    _helper.log(bmcl::LogLevel::Warning, fmt::format("Тм-кадр в прошивке не найден при декодировке! {}", name));
            }
          , [this, name](const caf::error& e)
            {
                _helper.log(bmcl::LogLevel::Warning, fmt::format("Ошибка при подписке на тм-кадр {}: {}", name, system().render(e)));
            }
        );
    };

    sub(v->statusMsgGrpElectSub(), "grp.elect");
    sub(v->statusMsgGrpAllSub(), "grp.all");
    sub(v->statusMsgGrpLeaderSub(), "grp.leader");
    sub(v->statusMsgGrpMembersSub(), "grp.members");

    sub(v->statusMsgNavAllSub(), "nav.all");
    sub(v->statusMsgNavGpsStateSub(), "nav.gpsstate");
    sub(v->statusMsgNavGpsAllSub(), "nav.gpsall");
    sub(v->statusMsgNavStateSub(), "nav.state");

    sub(v->statusMsgFcuCalibrationSub(), "fcu.calibrationState");
    sub(v->statusMsgFcuCommonCalibrationSub(), "fcu.commonCalibration");
    sub(v->statusMsgFcuFlightModesSub(), "fcu.flightModes");
    sub(v->statusMsgFcuRcChannelsSub(), "fcu.rcChannels");
}

void Device::set_firmware(const mccmsg::IFirmwarePtr& frm)
{
    FirmwarePtr r = bmcl::dynamic_pointer_cast<const Firmware>(frm);
    if (r == nullptr)
    {
        BMCL_WARNING() << "Запрос прошивки из БД вернул некорректный тип прошивки! Игнорируем!";
        return;
    }
    _pkg = r;
    send(_gc, ::photon::SetProjectAtom::value, r->pkg()); //поддерживаем только обмен с цвм
    subscribeForTm();
}

void Device::sendCmd(bmcl::Bytes bytes)
{
    auto rp = make_response_promise();

    if (!_isConnected)
    {
        rp.deliver(mccmsg::make_error(mccmsg::Error::NoChannelAvailable));
        return;
    }
    if (!_stats._isActive)
    {
        rp.deliver(mccmsg::make_error(mccmsg::Error::DeviceInactive));
        return;
    }

    ::photon::PacketRequest req(bytes, ::photon::StreamType::Cmd);

    request(_gc, caf::infinite, ::photon::SendReliablePacketAtom::value, std::move(req)).then
    (
        [this, rp](const ::photon::PacketResponse& response) mutable
        {
            if (response.type == ::photon::ReceiptType::Ok)
                rp.deliver(caf::unit);
            else
                rp.deliver(mccmsg::make_error(mccmsg::Error::CmdFailed, std::to_string((int)response.type)));
        }
      , [this, rp](const caf::error& e) mutable
        {
            rp.deliver(e);
        }
    );
}

void Device::sendCmd(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, bmcl::Bytes bytes, bool needPhotonRespHack)
{
    if (!_isConnected)
    {
        cmd->sendFailed(mccmsg::Error::NoChannelAvailable);
        return;
    }
    if (!_stats._isActive)
    {
        cmd->sendFailed(mccmsg::Error::DeviceInactive);
        return;
    }

    bmcl::Bytes payload;
    bmcl::Buffer dest;
    if (group.isSome())
    {
        ::photon::CoderState state(::photon::OnboardTime::now());
        if (!_pkg->pkg()->interface()->encodeCmdGrpExecute(mccmsg::groupToId(group.unwrap()), bytes.toStdVector(), &dest, &state))
        {
            cmd->sendFailed("не удалось закодировать команду для группы");
            return;
        }
        payload = dest;
    }
    else
    {
        payload = bytes;
    }

    request(_gc, caf::infinite, ::photon::SendReliablePacketAtom::value, ::photon::PacketRequest(payload, ::photon::StreamType::Cmd)).then
    (
        [this, cmd, needPhotonRespHack](const ::photon::PacketResponse& response) mutable
        {
            if (response.type == ::photon::ReceiptType::Ok)
                cmd->sendDone();
            else
                cmd->sendFailed(std::to_string((int)response.type));

            if (needPhotonRespHack)
                _helper.sendTrait(new TmPacketResponsePhoton(_id.device(), response));
        }
      , [this, cmd](caf::error& e) mutable
        {
            cmd->sendFailed(std::move(e));
        }
    );
}

void Device::sendCmd(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const ::photon::PacketRequest& r, bool needPhotonRespHack)
{
    if (!_isConnected)
    {
        cmd->sendFailed(mccmsg::Error::NoChannelAvailable);
        return;
    }
    if (!_stats._isActive)
    {
        cmd->sendFailed(mccmsg::Error::DeviceInactive);
        return;
    }

    bmcl::Bytes bytes = r.payload.view();
    bmcl::Bytes payload;
    bmcl::Buffer dest;
    if (group.isSome())
    {
        ::photon::CoderState state(::photon::OnboardTime::now());
        if (!_pkg->pkg()->interface()->encodeCmdGrpExecute(mccmsg::groupToId(group.unwrap()), bytes.toStdVector(), &dest, &state))
        {
            cmd->sendFailed("не удалось закодировать команду для группы");
            return;
        }
        payload = dest;
    }
    else
    {
        payload = bytes;
    }

    ::photon::PacketRequest req(r.requestUuid, payload, r.streamType);

    request(_gc, caf::infinite, ::photon::SendReliablePacketAtom::value, std::move(req)).then
    (
        [this, cmd, needPhotonRespHack](const ::photon::PacketResponse& response) mutable
        {
            if (response.type == ::photon::ReceiptType::Ok)
                cmd->sendDone();
            else
                cmd->sendFailed(std::to_string((int)response.type));

            if (needPhotonRespHack)
                _helper.sendTrait(new TmPacketResponsePhoton(_id.device(), response));
        }
      , [this, cmd](caf::error& e) mutable
        {
            cmd->sendFailed(std::move(e));
        }
    );
}

void copyCalibrationSideStatus(const photongen::fcu::CalibrationSideStatus& from, mccmsg::CalibrationSideStatus& to)
{
    to.done = from.done();
    to.inProgress = from.inProgress();
    to.rotate = from.rotate();
    to.visible = from.visible();
}

caf::behavior Device::make_behavior()
{
    set_down_handler([this](caf::down_msg& dm)
    {
    });

    set_default_handler(caf::print_and_drop);

    set_error_handler([this](caf::scheduled_actor* ptr, caf::error& x) {
        std::string str = system().render(x);
        return caf::sec::unexpected_message;
    });

    request(_core, caf::infinite, mccmsg::makeReq(new mccmsg::device::Description_Request(_id.device()))).then
    (
        [this](const mccmsg::device::Description_ResponsePtr& rep)
        {
            auto frm = rep->data()->firmware();
            if (frm.isNone())
                return;
            request(_core, caf::infinite, mccmsg::makeReq(new mccmsg::firmware::Description_Request(frm.unwrap()))).then
            (
                [this](const mccmsg::firmware::Description_ResponsePtr& rep)
                {
                    set_firmware(rep->data()->frm());
                }
              , [this](caf::error& err)
                {
                }
            );
        }
      , [this](caf::error& err)
        {
            BMCL_WARNING() << "Не удалось получить описание устройства из бд. Не зарегистрировано? Останавливаем обмен";
            quit();
        }
    );

    send(this, timerAtom::value);

    return {
        [this](const mccmsg::device::Activate_RequestPtr msg) -> caf::result<mccmsg::device::Activate_ResponsePtr>
        {
            _stats._isActive = msg->data()._state;
            state_changed();

//             _helper.sendTrait(new mccmsg::TmRoute(_id.device(), mccmsg::Waypoints(), mccmsg::RouteProperties(50, 1, "тестовый", false, false, false, false, false, false)));
//             _helper.sendTrait(new mccmsg::TmRoutesList(_id.device(), {route.properties}));
            return mccmsg::make<mccmsg::device::Activate_Response>(msg.get());
        }
      , [this](mccnet::connected_atom)
        {
            _isConnected = true;
            state_changed();
        }
      , [this](mccnet::disconnected_atom)
        {
            _isConnected = false;
            state_changed();
        }
      , [this](mccmsg::PacketPtr& pkt)
        {
            _stats._rcvd.add(pkt->size(), 1);
            send(_gc, ::photon::RecvDataAtom::value, bmcl::SharedBytes::create(pkt->data(), pkt->size()));
        }
      , [this](mccnet::send_cmd_atom, const bmcl::SharedBytes& bytes)
        {
            sendCmd(bytes.view());
        }
      , [this](::photon::RecvDataAtom, const bmcl::SharedBytes& data)
        {
            if (!_stats.isActive() || !_isConnected)
                return;
            mccmsg::PacketPtr pkt = new mccmsg::Packet(data.data(), data.size());
            _stats._sent.add(data.size(), 1);
            send(_broker, mccnet::req_atom::value, _id.device(), pkt);
        }
      , [this](::photon::SetProjectAtom, const ::photon::ProjectUpdate::ConstPointer& update)
        {
            mccmsg::ProtocolValue pv(_id.protocol(), Firmware::getName(update->serialize()));
            _pkg = new Firmware(pv, update, mccmsg::PropertyDescriptionPtrs(), mccmsg::PropertyDescriptionPtrs());
            mccmsg::IFirmwarePtr frm = _pkg;
            subscribeForTm();

            auto dscr = bmcl::makeRc<const mccmsg::FirmwareDescriptionObj>(mccmsg::Firmware::createNil(), pv, frm);
            request(_core, caf::infinite, mccmsg::makeReq(new mccmsg::firmware::Register_Request(std::move(dscr)))).then
            (
                [this](const mccmsg::firmware::Register_ResponsePtr& rep)
                {
                    mccmsg::DeviceDescription d = new mccmsg::DeviceDescriptionObj(_id.device(), "", "", _id, bmcl::None, bmcl::SharedBytes(), rep->data()->name(), false, false);
                    mccmsg::device::Updater u(std::move(d), { mccmsg::Field::Firmware });

                    request(_core, caf::infinite, mccmsg::makeReq(new mccmsg::device::Update_Request(std::move(u)))).then
                    (
                        [this](const mccmsg::device::Update_ResponsePtr)
                        {
                        }
                      , [this](caf::error)
                        {
                        }
                    );
                }
              , [this, frm](caf::error& err)
                {
                    BMCL_WARNING() << "Не удалось зарегистрировать прошивку устройства! пытаемся загрузить из БД";
                    restore_firmware(frm->id());
                }
            );
        }
      , [this](::photon::UpdateTmViewAtom, const bmcl::Rc<::photon::NodeViewUpdater>& statusUpdate, const bmcl::Rc<::photon::NodeViewUpdater>& eventUpdate, const bmcl::Rc<::photon::NodeViewUpdater>& statsUpdate)
        {
            publishTmEventUpdates(&_helper, eventUpdate->updates());
            _helper.sendTrait(new TmUpdateSES(_id.device(), statusUpdate, eventUpdate, statsUpdate));
        }
      , [this](::photon::SetTmViewAtom, bmcl::Rc<::photon::NodeView> statusView, bmcl::Rc<::photon::NodeView> eventView, bmcl::Rc<::photon::NodeView> statsView)
        {
            _helper.sendTrait(new TmView(_id.device(), _pkg, statusView, eventView, statsView));
        }
      , [this](::photon::UpdateTmParams, ::photon::TmParamUpdate& params)
        {
            switch (params.kind())
            {
            case ::photon::TmParamKind::RoutesInfo:
            {
                const ::photon::AllRoutesInfo& ps = params.as<::photon::AllRoutesInfo>();
                mccmsg::RoutesProperties prs;
                for (const auto& i : ps.info)
                {
                    mccmsg::BitFlags fs;
                    fs.set(mccmsg::RouteFlag::Closed, i.isClosed);
                    fs.set(mccmsg::RouteFlag::Inverted, i.isInverted);
                    prs.push_back(mccmsg::RouteProperties(i.maxSize, i.id, std::to_string(i.id), fs, i.activePoint.into<std::size_t>()));

                    ::photon::DownloadRouteGcCmd c;
                    c.id = i.id;
                    request(_gc, caf::infinite, ::photon::SendGcCommandAtom::value, ::photon::GcCmd(c)).then
                    (
                        [this](caf::unit_t) {}
                      , [this](caf::error& e) {}
                    );
                }
                _helper.sendTrait(new mccmsg::TmRoutesList(_id.device(), std::move(prs), ps.activeRoute.into<uint32_t>()));
                break;
            }
            case ::photon::TmParamKind::Route:
            {
                const ::photon::RouteTmParam& rtmp = params.as<::photon::RouteTmParam>();
                const auto& i = rtmp.route.info;
                mccmsg::Waypoints ws;
                for (const auto& j : rtmp.route.waypoints)
                {
                    mccgeo::Position pos(j.position.latLon.latitude, j.position.latLon.longitude, j.position.altitude);
                    bmcl::Option<uint32_t> sleep;
                    mccmsg::Properties wps;

                    switch (j.action.kind())
                    {
                    case ::photon::WaypointActionKind::Loop:
//                         wps.set(mccmsg::WaypointProperties::Loop, true);
                        break;
                    case ::photon::WaypointActionKind::Reynolds:
//                        wps.set(mccmsg::WaypointProperties::Reynolds, true);
                        break;
                    case ::photon::WaypointActionKind::Snake:
/*                        wps.set(mccmsg::WaypointProperties::Snake, true);*/
                        break;
                    case ::photon::WaypointActionKind::Sleep:
                        sleep = j.action.as<::photon::SleepWaypointAction>().timeout;
                        break;
                    case ::photon::WaypointActionKind::Formation:
//                         mccmsg::Formation formation;
//                         wps.set(mccmsg::WaypointProperties::Formation, true);
//                         auto ets = j.action.as<::photon::FormationWaypointAction>();
//                         for (const auto& k : ets.entries)
//                             formation.addEntry(mccmsg::FormationEntry(i.id, mccgeo::Vector3D(k.pos.x, k.pos.y, k.pos.z)));
                        break;
                    case ::photon::WaypointActionKind::None:
                        //HACK
                        break;
                    }

//                    mccmsg::Waypoint point(pos, j.speed.unwrapOr(0.0), 0.0, /*wps, sleep, formation*/);
//                    ws.push_back(point);
                }
                mccmsg::BitFlags fs;
                fs.set(mccmsg::RouteFlag::Closed, i.isClosed);
                fs.set(mccmsg::RouteFlag::Inverted, i.isInverted);
                mccmsg::RouteProperties ps(i.maxSize, i.id, std::to_string(i.id), fs, i.activePoint.into<std::size_t>());
                _helper.sendTrait(new mccmsg::TmRoute(_id.device(), mccmsg::Route(std::move(ws), std::move(ps))));
                break;
            }
            default:
                //HACK
                break;
            }
        }
      , [this](const ::photon::Value& value, const std::string& path)
        {
             mccmsg::NetVariant netValue;
             if (value.isA(::photon::ValueKind::Double))
                 netValue = value.asDouble();
             else if (value.isA(::photon::ValueKind::Signed))
                 netValue = value.asSigned();
             else if (value.isA(::photon::ValueKind::Unsigned))
                 netValue = value.asUnsigned();
             else
                 return;

             auto sepIt = path.find_last_of('.');
             if (sepIt == std::string::npos)
             {
                 assert(false);
                 return;
             }
             auto trait = path.substr(0, sepIt);
             auto varId = path.substr(sepIt + 1);
//              mccmsg::TmParams params = {
//                 { trait, varId, netValue },
//              };
//             _helper.sendTm(std::move(params));
        }
      , [this](::photon::LogAtom, const std::string& event)
        {
            _helper.log_text(bmcl::LogLevel::Info, event);
        }
      , [this](::photon::FirmwareDownloadStartedEventAtom)
        {
            //_stats._regState = 0;
        }
      , [this](::photon::FirmwareDownloadFinishedEventAtom)
        {
            _frm.load = _frm.size;
            //_stats._regState = 100;
        }
      , [this](::photon::ExchangeErrorEventAtom, const std::string& msg)
        {
            _helper.log_text(bmcl::LogLevel::Warning, msg);
        }
      , [this](::photon::FirmwareHashDownloadedEventAtom, const std::string& device, const bmcl::SharedBytes& frm_name)
        {
            _frm.name = Firmware::getNameFromHash(frm_name.view());
            restore_firmware(mccmsg::ProtocolValue(_helper.protocol(), _frm.name));
        }
      , [this](::photon::FirmwareSizeRecievedEventAtom, std::size_t size)
        {
            _frm.size = size;
            //_stats._regState = 0;
        }
      , [this](::photon::FirmwareProgressEventAtom, std::size_t progress, std::size_t size)
        {
            _frm.load = progress;
            assert(_frm.size == size);
            if (size == 0)
            {
                assert(false);
                //_stats._regState = 0;
                return;
            }
            //_stats._regState = progress*100 / size;
        }
      , [this](::photon::FirmwareErrorEventAtom, const std::string& event)
        {
            _helper.log_text(bmcl::LogLevel::Warning, event);
        }
      , [this](::photon::FirmwareStartCmdPassedEventAtom)
        {
        }
      , [this](::photon::FirmwareStartCmdSentEventAtom)
        {
        }
      , [this](const mccmsg::device::UpdatedPtr&)
        {
        }
      , [this](const mccmsg::DevReqPtr& msg)
        {
            Visitor visitor{ this, bmcl::None, bmcl::makeRc<mccnet::Cmd>(make_response_promise(), msg) };
            msg->visit(&visitor);
            return caf::delegated<void>();
        }
      , [this](const mccmsg::CancelPtr&)
        {
            assert(false);
        }
      , [this](timerAtom)
        {
            _helper.sendStats(_stats);
            delayed_send(this, std::chrono::milliseconds(100), timerAtom::value);
        }
      , [this](mccnet::group_cmd_new, const mccmsg::Group& group, std::vector<std::size_t>& devs) -> caf::result<void>
        {
            if (_pkg.isNull())
                return mccmsg::make_error(mccmsg::Error::FirmwareUnknown);
            bmcl::Buffer dest;
            ::photon::CoderState state(::photon::OnboardTime::now());
            if (!_pkg->pkg()->interface()->encodeCmdGrpCreateGroup(mccmsg::groupToId(group), devs, &dest, &state))
                return mccmsg::make_error(mccmsg::Error::CmdFailed);
            sendCmd(dest.asBytes());
            return caf::delegated<void>();
        }
      , [this](mccnet::group_cmd_del, const mccmsg::Group& group) -> caf::result<void>
        {
            if (_pkg.isNull())
                return mccmsg::make_error(mccmsg::Error::FirmwareUnknown);
            bmcl::Buffer dest;
            ::photon::CoderState state(::photon::OnboardTime::now());
            if (!_pkg->pkg()->interface()->encodeCmdGrpDeleteGroup(mccmsg::groupToId(group), &dest, &state))
                return mccmsg::make_error(mccmsg::Error::CmdFailed);
            sendCmd(dest.asBytes());
            return caf::delegated<void>();
        }
      , [this](mccnet::group_cmd_att, const mccmsg::Group& group, std::size_t dev) -> caf::result<void>
        {
            if (_pkg.isNull())
                return mccmsg::make_error(mccmsg::Error::FirmwareUnknown);
            bmcl::Buffer dest;
            ::photon::CoderState state(::photon::OnboardTime::now());
            if (!_pkg->pkg()->interface()->encodeCmdGrpAddMember(mccmsg::groupToId(group), dev, &dest, &state))
                return mccmsg::make_error(mccmsg::Error::CmdFailed);
            sendCmd(dest.asBytes());
            return caf::delegated<void>();
        }
      , [this](mccnet::group_cmd_det, const mccmsg::Group& group, std::size_t dev) -> caf::result<void>
        {
            if (_pkg.isNull())
                return mccmsg::make_error(mccmsg::Error::FirmwareUnknown);
            bmcl::Buffer dest;
            ::photon::CoderState state(::photon::OnboardTime::now());
            if (!_pkg->pkg()->interface()->encodeCmdGrpRemoveMember(mccmsg::groupToId(group), dev, &dest, &state))
                return mccmsg::make_error(mccmsg::Error::CmdFailed);
            sendCmd(dest.asBytes());
            return caf::delegated<void>();
        }
      , [this](mccnet::group_cmd, const mccmsg::Group& group, const mccmsg::DevReqPtr& msg) -> caf::result<void>
        {
            if (_pkg.isNull())
                return mccmsg::make_error(mccmsg::Error::FirmwareUnknown);

            Visitor visitor{ this, group, bmcl::makeRc<mccnet::Cmd>(make_response_promise(), msg) };
            msg->visit(&visitor);
            return caf::delegated<void>();
        }
      , [this](const ::photon::NumberedSub& sub, const bmcl::SharedBytes& value)
        {
            _helper.sendTrait(new TmUpdateSub(_id.device(), sub, value));

            bmcl::MemReader reader(value.view());
            ::photon::CoderState state(::photon::OnboardTime::now());

            auto v = _pkg->pkg()->interface();

            if (sub == v->statusMsgNavAllSub())
            {
                photongen::nav::statuses::All msg;
                if (photongenDeserialize(&msg, &reader, &state))
                {
//                     _motion.position = mccgeo::Position(msg.latLon.latitude(), msg.latLon.longitude(), msg.altitude);
//                     _motion.velocity = mccgeo::Position(msg.velocity.x(), msg.velocity.y(), msg.velocity.z());
//                     _motion.speed = std::hypot(std::hypot(msg.velocity.x(), msg.velocity.y()), msg.velocity.z());
//                     _motion.orientation = mccgeo::Attitude(msg.orientation.heading(), msg.orientation.pitch(), msg.orientation.roll());
//                     _helper.sendTrait(new mccmsg::TmMotion(_id.device(), _motion));
                }
            }
            else if (sub == v->statusMsgNavGpsStateSub())
            {
                photongen::nav::statuses::GpsState msg;
                if (photongenDeserialize(&msg, &reader, &state))
                {
                    //_helper.sendTrait(new mccmsg::TmGpsStatus(_id.device(), (mccmsg::GpsFixType)msg.gpsState_fixType, msg.gpsState_satellitesVisible));
                }
            }
            else if (sub == v->statusMsgNavGpsAllSub())
            {
                photongen::nav::statuses::GpsAll msg;
                if (photongenDeserialize(&msg, &reader, &state))
                {
                    mccmsg::GpsSats sats;
                    sats.reserve(msg.gpsState.satellites().size());
                    for (const auto& i : msg.gpsState.satellites())
                    {
                        sats.emplace_back(mccmsg::GpsSat(i.id(), i.elevation(), i.azimuth(), i.snr(), i.used()));
                    }
                    //_helper.sendTrait(new mccmsg::TmGpsSats(_id.device(), std::move(sats)));
                }
            }
            else if (sub == v->statusMsgGrpAllSub())
            {
                photongen::grp::statuses::All msg;
                if (photongenDeserialize(&msg, &reader, &state))
                {
                    if (msg.group.isSome())
                    {
                        send(_group, mccnet::group_stat::value, _id.device(), mccnet::GroupState(msg.group.unwrap(), msg.term, msg.leader, msg.members));
                        send(_group, mccnet::group_stat_dev::value, _id.device(), mccnet::GroupDeviceState(msg.group, msg.leader, msg.term));
                    }
                }
            }
            else if (sub == v->statusMsgGrpElectSub())
            {
                photongen::grp::statuses::Elect msg;
                if (photongenDeserialize(&msg, &reader, &state))
                {
                    if (msg.group.isSome())
                    {
                        send(_group, mccnet::group_stat_dev::value, _id.device(), mccnet::GroupDeviceState(msg.group, msg.leader, msg.term));
                    }
                }
            }
            else if (sub == v->statusMsgGrpMembersSub())
            {
                photongen::grp::statuses::Members msg;
                if (photongenDeserialize(&msg, &reader, &state))
                {
                    if (msg.group.isSome())
                    {
                        send(_group, mccnet::group_stat::value, _id.device(), mccnet::GroupState(msg.group.unwrap(), msg.term, msg.leader, msg.members));
                    }
                }
            }
            else if (sub == v->statusMsgGrpLeaderSub())
            {
                photongen::grp::statuses::Leader msg;
                if (photongenDeserialize(&msg, &reader, &state))
                {
                    if (msg.group.isSome())
                    {
                        send(_group, mccnet::group_stat_dev::value, _id.device(), mccnet::GroupDeviceState(msg.group, msg.leader, msg.term));
                    }
                }
            }
            else if (sub == v->statusMsgFcuCalibrationSub())
            {
                photongen::fcu::statuses::Calibration msg;
                if (photongenDeserialize(&msg, &reader, &state))
                {
                    mccmsg::Calibration calibrationStatus;

                    calibrationStatus._sensor = (mccmsg::CalibrationSensor)msg.calibrationState.sensor();

                    calibrationStatus.waitingForCancel = false;
                    calibrationStatus.failed = false;
                    calibrationStatus.done = false;

                    switch (msg.calibrationState.status())
                    {
                        case photongen::fcu::CalibrationStatus::NotStarted:
                            break;
                        case photongen::fcu::CalibrationStatus::Failed:
                            calibrationStatus.failed = true;
                            break;
                        case photongen::fcu::CalibrationStatus::Done:
                            calibrationStatus.done = true;
                            break;
                        case photongen::fcu::CalibrationStatus::CancelPending:
                            calibrationStatus.waitingForCancel = true;
                            break;
                        case photongen::fcu::CalibrationStatus::Cancelled:
                            calibrationStatus.waitingForCancel = true;
                            break;
                        default:
                            break;
                    }

                    copyCalibrationSideStatus(msg.calibrationState.calDownSide(),       calibrationStatus.calDownSide);
                    copyCalibrationSideStatus(msg.calibrationState.calUpsideDownSide(), calibrationStatus.calUpsideDownSide);
                    copyCalibrationSideStatus(msg.calibrationState.calLeftSide(),       calibrationStatus.calLeftSide);
                    copyCalibrationSideStatus(msg.calibrationState.calRightSide(),      calibrationStatus.calRightSide);
                    copyCalibrationSideStatus(msg.calibrationState.calNoseDownSide(),   calibrationStatus.calNoseDownSide);
                    copyCalibrationSideStatus(msg.calibrationState.calTailDownSide(),   calibrationStatus.calTailDownSide);

                    if (calibrationStatus.isMagnetometer())
                    {
                        calibrationStatus.message = "Rotate the vehicle continuously as shown in the diagram until marked as Completed";
                    }
                    else
                    {
                        calibrationStatus.message = "Hold still in the current orientation";
                    }
                    calibrationStatus.progress = msg.calibrationState.progress();
                    calibrationStatus.nextEnabled = msg.calibrationState.nextEnabled();
                    calibrationStatus.skipEnabled = msg.calibrationState.skipEnabled();

                    if (calibrationStatus.progress == 100)
                    {
                        calibrationStatus.message = "Completed";
                    }
                    _helper.sendTrait(new mccmsg::TmCalibration(_id.device(), std::move(calibrationStatus)));
                }
            }
            else if (sub == v->statusMsgFcuCommonCalibrationSub())
            {
                photongen::fcu::statuses::CommonCalibration msg;
                if (photongenDeserialize(&msg, &reader, &state))
                {
                    mccmsg::TmCommonCalibrationStatus* commonStatusPtr = new mccmsg::TmCommonCalibrationStatus(_id.device());
                    mccmsg::TmCommonCalibrationStatus& commonStatus = *commonStatusPtr;

                    commonStatus.accelerometer = msg.commonCalibrationState.accelerometer();
                    commonStatus.magnetometer = msg.commonCalibrationState.magnetometer();
                    commonStatus.gyroscope = msg.commonCalibrationState.gyroscope();
                    commonStatus.level = msg.commonCalibrationState.level();
                    commonStatus.esc = msg.commonCalibrationState.esc();
                    commonStatus.radio = msg.commonCalibrationState.radio();
                    _helper.sendTrait(commonStatusPtr);
                }
            }
            else if (sub == v->statusMsgFcuFlightModesSub())
            {
                photongen::fcu::statuses::FlightModes msg;
                if (photongenDeserialize(&msg, &reader, &state))
                {

                    mccmsg::Calibration flightModesCalibration;

                    flightModesCalibration._sensor = mccmsg::CalibrationSensor::FlightModes;
                    flightModesCalibration.flightModes.killSwitchChannel = (int32_t)msg.flightModes.killSwitchChannel();
                    flightModesCalibration.flightModes.offboardSwitchChannel = (int32_t)msg.flightModes.offboardSwitchChannel();
                    flightModesCalibration.flightModes.returnSwitchChannel = (int32_t)msg.flightModes.returnSwitchChannel();
                    flightModesCalibration.flightModes.flightModeChannel = (int32_t)msg.flightModes.flightModeChannel();
                    flightModesCalibration.flightModes.mode1 = (int32_t)msg.flightModes.modes()[0];
                    flightModesCalibration.flightModes.mode2 = (int32_t)msg.flightModes.modes()[1];
                    flightModesCalibration.flightModes.mode3 = (int32_t)msg.flightModes.modes()[2];
                    flightModesCalibration.flightModes.mode4 = (int32_t)msg.flightModes.modes()[3];
                    flightModesCalibration.flightModes.mode5 = (int32_t)msg.flightModes.modes()[4];
                    flightModesCalibration.flightModes.mode6 = (int32_t)msg.flightModes.modes()[5];
                    _helper.sendTrait(new mccmsg::TmCalibration(_id.device(), std::move(flightModesCalibration)));
                }
            }
            else if (sub == v->statusMsgFcuRcChannelsSub())
            {
                photongen::fcu::statuses::RcChannels msg;
                if (photongenDeserialize(&msg, &reader, &state))
                {
                    send(_radioCalibration, rc_channels_atom::value, msg.rcChannels);
                }
            }
        }
    };
}

void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdParamList& msg)
{
    using mccmsg::NetVariantType;
    using ::photon::Value;

    std::vector<Value> args;
    for (auto p : msg.params())
    {
        switch (p.type())
        {
            case NetVariantType::Uint:
                args.emplace_back(Value::makeUnsigned(p.asUint()));
                break;
            case NetVariantType::Int:
                args.emplace_back(Value::makeSigned(p.asInt()));
                break;
            case NetVariantType::Double:
                args.emplace_back(Value::makeDouble(p.asDouble()));
                break;
            case NetVariantType::Float:
                args.emplace_back(Value::makeDouble(p.asFloat()));
                break;
            case NetVariantType::String:
                args.emplace_back(Value::makeString(p.asString()));
                break;
            default:
            {
                cmd->sendFailed(mccmsg::Error::CmdUnknown);
                return;
            }
        }
    }

    if (_pkg.isNull())
    {
        cmd->sendFailed(mccmsg::Error::FirmwareUnknown);
        return;
    }

    auto mod = _pkg->pkg()->project()->package()->moduleWithName(msg.trait());
    if (mod.isNone())
    {
        cmd->sendFailed(mccmsg::Error::CmdUnknownTrait);
        return;
    }

    auto comp = mod.unwrap()->component();
    if (comp.isNone())
    {
        cmd->sendFailed(mccmsg::Error::CmdUnknownTrait);
        return;
    }

    auto comp_cmd = comp.unwrap()->cmdWithName(msg.command());
    if (comp_cmd.isNone())
    {
        cmd->sendFailed(mccmsg::Error::CmdUnknown);
        return;
    }

    decode::Rc<::photon::CmdNode> cmdNode = new ::photon::CmdNode(comp.unwrap(), comp_cmd.unwrap(), _pkg->pkg()->cache(), bmcl::None);
    if (!cmdNode->setValues(args))
    {
        cmd->sendFailed("Не удалось задать параметры");
        assert(false);
        return;
    }

    ::photon::CoderState state(::photon::OnboardTime::now());
    bmcl::Buffer dest;
    dest.reserve(2048);
    if (!cmdNode->encode(&state, &dest))
    {
        cmd->sendFailed("Не удалось закодировать команду");
        return;
    }
    sendCmd(group, std::move(cmd), dest.asBytes());
}

void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdPacketRequest& msg)
{
    assert(false); //downcast
    const CmdPacketPhoton* p = dynamic_cast<const CmdPacketPhoton*>(&msg);
    assert(p);
    assert(p->packet().streamType == ::photon::StreamType::Cmd);
    sendCmd(group, std::move(cmd), p->packet(), p->hasResponse());
}

void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteSet& msg)
{
    const auto& r = msg.route();

    ::photon::UploadRouteGcCmd route;
    route.id = r.properties.name;
    route.isClosed = r.properties.flags.get(mccmsg::RouteFlag::Closed);
    route.isInverted = r.properties.flags.get(mccmsg::RouteFlag::Inverted);
    route.isReadOnly = r.properties.flags.get(mccmsg::RouteFlag::ReadOnly);
    route.activePoint = r.properties.nextWaypoint.into<uintmax_t>();

    for (const auto& i : r.waypoints)
    {
        ::photon::Waypoint point;
        point.position.altitude = i.position.altitude();
        point.position.latLon.latitude = i.position.latitude();
        point.position.latLon.longitude = i.position.longitude();
        point.speed = i.speed;

//         if (i.properties.get(mccmsg::WaypointProperties::Snake))
//             point.action = ::photon::SnakeWaypointAction();
//         else if (i.properties.get(mccmsg::WaypointProperties::Reynolds))
//             point.action = ::photon::ReynoldsWaypointAction();
//         else if (i.properties.get(mccmsg::WaypointProperties::Loop))
//             point.action = ::photon::LoopWaypointAction();
//         else if (i.properties.get(mccmsg::WaypointProperties::Formation))
//         {
//             ::photon::FormationWaypointAction tmp;
//             for (const auto& j : i.formation.entries())
//             {
//                 ::photon::FormationEntry ety;
//                 ety.id = j.id();
//                 mccgeo::Vector3D position = j.position();
//                 ety.pos.x = position.x();
//                 ety.pos.y = position.y();
//                 ety.pos.z = position.z();
//                 tmp.entries.emplace_back(ety);
//             }
//             point.action = tmp;
//         }
        route.waypoints.push_back(point);
    }

    bmcl::Rc<const mccmsg::CmdRouteSet> handle(&msg);
    request(_gc, caf::infinite, ::photon::SendGcCommandAtom::value, ::photon::GcCmd(std::move(route))).then
    (
        [this, cmd, handle](caf::unit_t) mutable
        {
            ::photon::DownloadRouteGcCmd c;
            c.id = handle->route().properties.name;
            request(_gc, caf::infinite, ::photon::SendGcCommandAtom::value, ::photon::GcCmd(c)).then([](caf::unit_t) { }, [](caf::error&) { });
        },
        [this, cmd](caf::error& e) mutable
        {
            cmd->sendFailed(std::move(e));
        }
    );
}

void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteSetNoActive& msg)
{
    if (_pkg.isNull())
    {
        cmd->sendFailed(mccmsg::Error::FirmwareUnknown);
        return;
    }
    bmcl::Buffer dest;
    ::photon::CoderState state(::photon::OnboardTime::now());
    if (!_pkg->pkg()->interface()->encodeCmdNavSetActiveRoute(::photongen::nav::OptionalRouteId(), &dest, &state))
    {
        cmd->sendFailed(mccmsg::Error::CmdFailed);
        return;
    }
    sendCmd(group, std::move(cmd), dest.asBytes());
}

void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteSetActive& msg)
{
    if (msg.point().isSome())
    {
        cmd->sendFailed(mccmsg::Error::NotImplemented);
        return;
    }

    if (_pkg.isNull())
    {
        cmd->sendFailed(mccmsg::Error::FirmwareUnknown);
        return;
    }
    bmcl::Buffer dest;
    ::photon::CoderState state(::photon::OnboardTime::now());
    ::photongen::nav::OptionalRouteId routeId;
    routeId.emplaceSome(::photongen::nav::OptionalRouteId::Some{ (uint64_t)msg.route() });
    if (!_pkg->pkg()->interface()->encodeCmdNavSetActiveRoute(routeId, &dest, &state))
    {
        cmd->sendFailed(mccmsg::Error::CmdFailed);
        return;
    }
    sendCmd(group, std::move(cmd), dest.asBytes());
}

void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteSetActivePoint& msg)
{
    if (_pkg.isNull())
    {
        cmd->sendFailed(mccmsg::Error::FirmwareUnknown);
        return;
    }
    bmcl::Buffer dest;
    ::photon::CoderState state(::photon::OnboardTime::now());
    ::photongen::nav::OptionalIndex index;
    if(msg.point().isSome())
        index.emplaceSome(::photongen::nav::OptionalIndex::Some{ (uint64_t)msg.point().unwrap() });

    if (!_pkg->pkg()->interface()->encodeCmdNavSetRouteActivePoint(msg.route(), index, &dest, &state))
    {
        cmd->sendFailed(mccmsg::Error::CmdFailed);
        return;
    }
    sendCmd(group, std::move(cmd), dest.asBytes());
}

void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteSetDirection& msg)
{
    if (_pkg.isNull())
    {
        cmd->sendFailed(mccmsg::Error::FirmwareUnknown);
        return;
    }
    bmcl::Buffer dest;
    ::photon::CoderState state(::photon::OnboardTime::now());
    if (!_pkg->pkg()->interface()->encodeCmdNavSetRouteInverted(msg.route(), !msg.isForward(), &dest, &state))
    {
        cmd->sendFailed(mccmsg::Error::CmdFailed);
        return;
    }
    sendCmd(group, std::move(cmd), dest.asBytes());
}

void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteGet& msg)
{
    ::photon::DownloadRouteGcCmd c;
    c.id = msg.route();

    request(_gc, caf::infinite, ::photon::SendGcCommandAtom::value, ::photon::GcCmd(c)).then
    (
        [this, cmd](caf::unit_t) mutable { cmd->sendDone(); }
      , [this, cmd](caf::error& e) mutable { cmd->sendFailed(std::move(e)); }
    );
}

void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteGetList& msg)
{
    request(_gc, caf::infinite, ::photon::SendGcCommandAtom::value, ::photon::GcCmd(::photon::DownloadRouteInfoGcCmd())).then
    (
        [this, cmd](caf::unit_t) mutable { cmd->sendDone(); }
      , [this, cmd](caf::error& e) mutable { cmd->sendFailed(std::move(e)); }
    );
}

void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdParamRead& msg)
{
    for (const auto& v : msg.vars())
    {
        request(_gc, caf::infinite, ::photon::SubscribeNamedTmAtom::value, msg.trait() + "." + v, this).then
        (
            [v, this](bool state)
            {
                if(!state)
                    _helper.log(bmcl::LogLevel::Warning, fmt::format("Ошибка при подписке на переменную {}!", v));
            }
          , [v, this](caf::error& e)
            {
                _helper.log(bmcl::LogLevel::Critical, fmt::format("Ошибка при подписке на переменную {}: {}!", system().render(e)));
            }
        );
    }

    cmd->sendDone();
}

void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdFileUpload& msg)
{
    auto reader = FileReader::create(cmd, msg.local(), msg.remote());
    if (reader.isErr())
        return;
    ::photon::UploadFileGcCmd c;
    c.id = std::atoi(msg.remote().c_str());
    c.reader = reader.take();
    request(_gc, caf::infinite, ::photon::SendGcCommandAtom::value, ::photon::GcCmd(::photon::UploadFileGcCmd(c))).then
    (
        [cmd](caf::unit_t)
        {
            cmd->sendDone();
        }
      , [cmd](caf::error& e)
        {
            cmd->sendFailed(std::move(e));
        }
    );
}

void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdCalibrationStart& msg)
{
    bmcl::Buffer dest;
    ::photon::CoderState state(::photon::OnboardTime::now());
    if (msg.specialCmd().isNone())
    {
        if (!_pkg->pkg()->interface()->encodeCmdFcuStartSensorCalibration((::photongen::fcu::Sensors)msg.component(), &dest, &state))
        {
            cmd->sendFailed(mccmsg::Error::CmdFailed);
            return;
        }
        sendCmd(group, std::move(cmd), dest.asBytes());
    }

    if (msg.component() == mccmsg::CalibrationSensor::Radio)
    {
        send(_radioCalibration, mccmsg::CmdCalibrationStartPtr(&msg));
    }
}

void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdCalibrationCancel& msg)
{
    bmcl::Buffer dest;
    ::photon::CoderState state(::photon::OnboardTime::now());
    if (!_pkg->pkg()->interface()->encodeCmdFcuCancelSensorCalibration(&dest, &state))
    {
        cmd->sendFailed(mccmsg::Error::CmdFailed);
        return;
    }
    sendCmd(group, std::move(cmd), dest.asBytes());
}

void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteCreate& msg) { cmd->sendFailed(mccmsg::Error::NotImplemented); }
void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteRemove& msg) { cmd->sendFailed(mccmsg::Error::NotImplemented); }
void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdRouteClear& msg) { cmd->sendFailed(mccmsg::Error::NotImplemented); }
void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdParamWrite& msg) { cmd->sendFailed(mccmsg::Error::NotImplemented); }
void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdFileGetList& msg) { cmd->sendFailed(mccmsg::Error::NotImplemented); }
void Device::execute(const bmcl::Option<mccmsg::Group>& group, mccnet::CmdPtr&& cmd, const mccmsg::CmdFileDownload& msg) { cmd->sendFailed(mccmsg::Error::NotImplemented); }
}
