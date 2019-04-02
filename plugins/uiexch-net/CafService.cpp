#include "CafService.h"

#include <QMetaType>
#include <QCoreApplication>
#include <QEvent>

#include <caf/scoped_actor.hpp>
#include <caf/event_based_actor.hpp>
#include <caf/others.hpp>

#include <bmcl/Logging.h>
#include <bmcl/MakeRc.h>

#include "mcc/net/Error.h"
#include "mcc/net/NetPlugin.h"

#include "mcc/Rc.h"
#include "mcc/msg/TmView.h"
#include "mcc/msg/ptr/NoteVisitor.h"
#include "mcc/msg/ptr/ReqVisitor.h"
#include "mcc/msg/ptr/All.h"
#include "mcc/plugin/PluginCache.h"

#define MSG_ALLOW_CAF(nmsp, name) CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::nmsp::name ## _RequestPtr); CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::nmsp::name ## _ResponsePtr)

MSG_ALLOW_CAF(channel, UnRegister);
MSG_ALLOW_CAF(channel, List);
MSG_ALLOW_CAF(channel, Description);
MSG_ALLOW_CAF(channel, DescriptionList);
MSG_ALLOW_CAF(channel, Activate);

MSG_ALLOW_CAF(device, UnRegister);
MSG_ALLOW_CAF(device, List);
MSG_ALLOW_CAF(device, Description);
MSG_ALLOW_CAF(device, DescriptionList);
MSG_ALLOW_CAF(device, Activate);
MSG_ALLOW_CAF(device, Connect);

MSG_ALLOW_CAF(protocol, Description);
MSG_ALLOW_CAF(protocol, DescriptionList);

MSG_ALLOW_CAF(firmware, Description);
MSG_ALLOW_CAF(advanced, ChannelAndDeviceRegister);

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::DbReqPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::DevReqPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::CmdRespAnyPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NotificationPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::RequestPtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Request_StatePtr);
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::CancelPtr);


constexpr const int BaseEventId = QEvent::User + 31337;

struct EventRep : public QEvent
{
    static constexpr const int EventId = BaseEventId + 0;
    mccmsg::RequestPtr req;
    mccmsg::ResponsePtr rep;
    inline EventRep(mccmsg::RequestPtr&& req, mccmsg::ResponsePtr&& rep) : QEvent(static_cast<QEvent::Type>(EventId)), req(std::move(req)), rep(std::move(rep)) {}
};

struct EventReqState : public QEvent
{
    static constexpr const int EventId = BaseEventId + 1;
    mccmsg::Request_StatePtr state;
    inline explicit EventReqState(const mccmsg::Request_StatePtr& state) : QEvent(static_cast<QEvent::Type>(EventId)), state(state) {}
};

struct EventErr: public QEvent
{
    static constexpr const int EventId = BaseEventId + 2;
    mccmsg::RequestPtr req;
    caf::error err;
    EventErr(mccmsg::RequestPtr&& req, caf::error&& err) : QEvent(static_cast<QEvent::Type>(EventId)), req(std::move(req)), err(std::move(err)) {}
    EventErr(mccmsg::RequestPtr&& req, const caf::error& err) : QEvent(static_cast<QEvent::Type>(EventId)), req(std::move(req)), err(err) {}
};

struct EventNote : public QEvent
{
    static constexpr const int EventId = BaseEventId + 3;
    mccmsg::NotificationPtr note;
    inline explicit EventNote(const mccmsg::NotificationPtr& note) : QEvent(static_cast<QEvent::Type>(EventId)), note(note) {}
};

class ReqRepHelper : public caf::event_based_actor
{
    friend class ReqVisitorH;
public:
    ReqRepHelper(caf::actor_config& cfg, const caf::actor& core, const std::string& name, CafService* self);
    inline const char* name() const override { return _name.c_str(); }
    caf::behavior make_behavior() override;
    void on_exit() override;
private:
    void make_visitor_call(const mccmsg::DbRequestPtr& req);
    CafService* _self;
    std::string _name;
    caf::actor _core;
};

ReqRepHelper::ReqRepHelper(caf::actor_config& cfg, const caf::actor& core, const std::string& name, CafService* self)
    : caf::event_based_actor(cfg), _self(self), _name(name), _core(core)
{
    monitor(core);
    auto grp = system().groups().get_local("notes");
    join(grp);
    send(_core, caf::atom("ui"));
}

void ReqRepHelper::on_exit()
{
    destroy(_core);
}

caf::behavior ReqRepHelper::make_behavior()
{
    set_default_handler(caf::print_and_drop);
    set_error_handler([this](caf::error& e)
    {
        auto r = current_sender();
        int id = r->address().id();
        std::string s = system().render(e);
        BMCL_DEBUG() << s << id;
    }
    );

    set_down_handler([this](caf::down_msg& dm)
    {
        if(dm.source != _core)
        {
            assert(false);
            return;
        }
        BMCL_DEBUG() << "ui: core down";
        destroy(_core);
        quit();
    });
    using namespace mccmsg;
    return{
        [this](const mccmsg::DbRequestPtr& req)
        {
            make_visitor_call(req);
        }
      , [this](const mccmsg::DevReqPtr& req)
        {
            request(_core, caf::infinite, req).then
            (
                [this, req](const mccmsg::CmdRespAnyPtr& rep) mutable
                {
                    qApp->postEvent(_self, new EventRep(std::move(req), rep));
                }
              , [this, req](const caf::error& err) mutable
                {
                    std::string s = system().render(err);
                    qApp->postEvent(_self, new EventErr(std::move(req), err));
                }
            );
        }
      , [this](const mccmsg::CancelPtr& req)
        {
            request(_core, caf::infinite, req).then
            (
                [this]()
                {
                }
              , [this](const caf::error& err)
                {
                }
            );
        }
      , [this](const mccmsg::Request_StatePtr& state)
        {
            qApp->postEvent(_self, new EventReqState(state));
        }
      , [this](const mccmsg::NotificationPtr& note)
        {
            qApp->postEvent(_self, new EventNote(note));
        }
      , [this](caf::error& e)
        {
            std::string s = system().render(e);
            BMCL_DEBUG() << s.c_str();
            assert(false);
        }
    };
}

class ReqVisitorH : public mccmsg::ReqVisitor
{
private:
    const mccmsg::DbRequestPtr& _ptr;
    ReqRepHelper* _self;
public:
    ReqVisitorH(const mccmsg::DbRequestPtr& ptr, ReqRepHelper* self)
        : _ptr(ptr), _self(self){}

    template<typename T>
    void visit_impl()
    {
        auto s = _self;
        typename T::RequestPtr req = bmcl::static_pointer_cast<const T>(_ptr);
        _self->request(_self->_core, caf::infinite, std::move(_ptr)).then
        (
            [s, req](typename T::ResponsePtr& rep) mutable
            {
                qApp->postEvent(s->_self, new EventRep(std::move(req), std::move(rep)));
            }
          , [s, req](caf::error& err) mutable
            {
                qApp->postEvent(s->_self, new EventErr(std::move(req), std::move(err)));
            }
        );
    }

#define VISIT_METHOD(t) void visit(const t*) override{ visit_impl<t>();}
#define VISIT_SET(nmsp) \
    VISIT_METHOD(mccmsg::nmsp::Register_Request);           \
    VISIT_METHOD(mccmsg::nmsp::UnRegister_Request);         \
    VISIT_METHOD(mccmsg::nmsp::List_Request);               \
    VISIT_METHOD(mccmsg::nmsp::Description_Request);        \
    VISIT_METHOD(mccmsg::nmsp::DescriptionS_Request);       \
    VISIT_METHOD(mccmsg::nmsp::DescriptionList_Request);    \
    VISIT_METHOD(mccmsg::nmsp::Update_Request);

    VISIT_SET(firmware);
    VISIT_SET(channel);
    VISIT_SET(device);
    VISIT_SET(deviceUi);
    VISIT_SET(protocol);
    VISIT_SET(radar);
    VISIT_SET(tmSession);

    VISIT_METHOD(mccmsg::advanced::ChannelAndDeviceRegister_Request);
    VISIT_METHOD(mccmsg::channel::Activate_Request);
    VISIT_METHOD(mccmsg::device::Activate_Request);
    VISIT_METHOD(mccmsg::device::Connect_Request);
    VISIT_METHOD(mccmsg::tm::Dump_Request);

#undef VISIT_SET
#undef VISIT_METHOD
};

void ReqRepHelper::make_visitor_call(const mccmsg::DbRequestPtr& req)
{
    auto ptr = req.get();
    ReqVisitorH visitor(std::move(req), this);
    ptr->visit(visitor);
}

class TmVisitor : public mccmsg::TmVisitor
{
private:
    CafService * service;
public:
    explicit TmVisitor(CafService* service, const mccmsg::tm::TmAnyPtr& tm) : service(service){}
    void visit(const mccmsg::TmMotion& tm) override { service->traitNavigationMotion(&tm); }
    void visit(const mccmsg::TmRoute& tm) override { service->traitRouteState(&tm); }
    void visit(const mccmsg::TmRoutesList& tm) override { service->traitRoutesList(&tm); }
    void visit(const mccmsg::TmCalibration& tm) override { service->traitCalibration(&tm); }
    void visit(const mccmsg::TmCommonCalibrationStatus& tm) override { service->traitCommonCalibrationStatus(&tm); }
    void visit(const mccmsg::TmGroupState& tm) override { service->traitGroupState(&tm); }
    void visit(const mccmsg::ITmView& tm) override { service->setTmView(&tm); }
    void visit(const mccmsg::ITmViewUpdate& tm) override { service->updateTmStatusView(&tm); }
    void visit(const mccmsg::ITmPacketResponse& tm) override { service->tmPaketResponse(&tm); }
};

class NoteVisitorX : public mccmsg::NoteVisitor
{
private:
    CafService* _self;
    mccmsg::NotificationPtr msg;
public:
    NoteVisitorX(CafService* self, const mccmsg::NotificationPtr& msg) : _self(self), msg(msg) {}
    void visit(const mccmsg::tm::Item* note) override
    {
        TmVisitor visitor(_self, note->data());
        note->data()->visit(&visitor);
    }

    void visit(const mccmsg::tm::Log* note) override { _self->log(note); }
    void visit(const mccmsg::channel::Updated* note) override { _self->channelUpdated(note->data()); }
    void visit(const mccmsg::device::Updated* note) override { _self->deviceUpdated(note->data()); }
    void visit(const mccmsg::device::Activated* note) override { _self->deviceActivated(note->data()._name, note->data()._state); }
    void visit(const mccmsg::protocol::Registered* note) override { _self->protocolRegistered(note->data()._name); }
    void visit(const mccmsg::device::Connected* note) override
    {
        if (note->data().isConnected())
            _self->deviceConnected(note->data().channel(), note->data().device());
        else
            _self->deviceDisconnected(note->data().channel(), note->data().device());
    }
    void visit(const mccmsg::channel::Registered* note) override
    {
        if (note->data()._state)
            _self->channelRegistered(note->data()._name);
        else
            _self->channelUnRegistered(note->data()._name);
    }
    void visit(const mccmsg::device::Registered* note) override
    {
        if (note->data()._state)
            _self->deviceRegistered(note->data()._name);
        else
            _self->deviceUnRegistered(note->data()._name);
    }
    void visit(const mccmsg::device::State* note) override
    {
        _self->deviceState(note->data());
    }
    void visit(const mccmsg::channel::State* note) override
    {
        _self->channelState(note->data());
    }
    void visit(const mccmsg::tmSession::Registered* note) override { _self->tmSessionRegistered(note->data()._name, note->data()._state); }
    void visit(const mccmsg::tmSession::Updated* note) override { _self->tmSessionUpdated(note->data()); }
};

void CafService::setCore(const caf::actor& core)
{
    _core = core;
    _helper = core->home_system().spawn<ReqRepHelper>(core, "ui.reqrep", this);
}

CafService::CafService()
{
    qRegisterMetaType<QVector<QString>>();
    qRegisterMetaType<mccmsg::DeviceDescription>();
    qRegisterMetaType<mccmsg::DeviceDescriptions>();
    qRegisterMetaType<mccmsg::ProtocolDescription>();
    qRegisterMetaType<mccmsg::ProtocolDescriptions>();
    qRegisterMetaType<mccmsg::ProtocolId>();
    qRegisterMetaType<mccmsg::ProtocolIds>();
    qRegisterMetaType<mccmsg::ChannelDescription>();
    qRegisterMetaType<mccmsg::ChannelDescriptions>();
    qRegisterMetaType<mccmsg::FirmwareDescription>();
    qRegisterMetaType<mccmsg::FirmwareDescriptions>();

    qRegisterMetaType<mccmsg::TmMotionPtr>();
    qRegisterMetaType<mccmsg::TmRoutePtr>();
    qRegisterMetaType<mccmsg::TmRoutesListPtr>();
    qRegisterMetaType<mccmsg::ErrorDscr>();
    qRegisterMetaType<bmcl::Rc<mccmsg::ITmViewUpdate>>();
    qRegisterMetaType<bmcl::Rc<mccmsg::ITmView>>();
}

CafService::~CafService()
{
    caf::anon_send_exit(_helper, caf::exit_reason::user_shutdown);
}

bmcl::Option<mccuav::ReqItem&> CafService::get(mccmsg::RequestId id)
{
    auto i = _requests.find(id);
    if (i == _requests.end())
        return bmcl::None;
    return i->second;
}

void CafService::onLog(bmcl::LogLevel logLevel, const mccmsg::Device& device, const std::string &text)
{
    caf::send_as(_helper, _core, mccmsg::makeNote(new mccmsg::tm::Log(logLevel, "mcc.ui", device, text)));
}

void CafService::onLog(const mccmsg::Device& device, const std::string& text)
{
    caf::send_as(_helper, _core, mccmsg::makeNote(new mccmsg::tm::Log(bmcl::LogLevel::Info, "mcc.ui", device, text)));
}

void CafService::onLog(const std::string& text)
{
    caf::send_as(_helper, _core, mccmsg::makeNote(new mccmsg::tm::Log(bmcl::LogLevel::Info, "mcc.ui", text)));
}

void CafService::onLog(bmcl::LogLevel logLevel, const std::string& text)
{
    caf::send_as(_helper, _core, mccmsg::makeNote(new mccmsg::tm::Log(logLevel, "mcc.ui", text)));
}

void CafService::addResponseHandler(mccuav::ReqItem&& item)
{
    if (!_helper)
    {
        item.onerror(mccmsg::ErrorDscr(mccmsg::Error::CoreDisconnected));
        return;
    }
    //BMCL_DEBUG() << "ui " << item.req()->nameXXX();
    auto id = item.req()->requestId();
    mccmsg::RequestPtr p = item.req();
    assert(_requests.find(id) == _requests.end());
    _requests.emplace(id, std::move(item));

    switch (p->kind())
    {
    case mccmsg::ReqKind::Db:
    {
        auto tmp = bmcl::static_pointer_cast<const mccmsg::DbReq>(p);
        caf::anon_send(_helper, tmp);
        //emit requestAdded(p);
    }
    break;
    case mccmsg::ReqKind::Dev:
    {
        auto tmp = bmcl::static_pointer_cast<const mccmsg::DevReq>(p);
        caf::anon_send(_helper, tmp);
        emit requestAdded(tmp);
    }
    break;
    default:
        break;
    }
}

bool CafService::event(QEvent* event)
{
    switch ((int)event->type())
    {
    case EventRep::EventId:
    {
        auto ptr = dynamic_cast<EventRep*>(event);
        if (!ptr)
            return QObject::event(event);

        auto id = ptr->req->requestId();
        const auto i = get(id);
        if (i.isNone())
            return true;

        i->onsuccess(std::move(ptr->rep));
        _requests.erase(id);
        if (ptr->req->kind() == mccmsg::ReqKind::Dev)
            emit requestRemoved(bmcl::static_pointer_cast<const mccmsg::DevReq>(ptr->req));
        return true;
    }
    case EventReqState::EventId:
    {
        auto ptr = dynamic_cast<EventReqState*>(event);
        if (!ptr)
            return QObject::event(event);

        bool isCmd = (ptr->state->request()->kind() == mccmsg::ReqKind::Dev);
        mccmsg::DevReqPtr req = bmcl::static_pointer_cast<const mccmsg::DevReq>(ptr->state->request());

        bmcl::Option<const mccuav::ReqItem&> i = get(req->requestId());
        if (i.isNone())
        {
            mccmsg::RequestPtr tmp = ptr->state->request();
            i = _requests.emplace(req->requestId(), mccuav::ReqItem(std::move(tmp), [](const mccmsg::ResponsePtr&){}, bmcl::None, bmcl::None, true) ).first->second;
            if (isCmd)
                emit requestAdded(req);
        }

        if (isCmd)
            emit requestStateChanged(req, ptr->state);

        i->onstate(std::move(ptr->state));

        if (ptr->state->result().isSome() && isCmd)
        {
            emit requestRemoved(req);
            _requests.erase(req->requestId());
        }

        return true;
    }
    case EventErr::EventId:
    {
        auto ptr = dynamic_cast<EventErr*>(event);
        if (!ptr)
            return QObject::event(event);

        auto id = ptr->req->requestId();
        const auto i = get(id);
        if (i.isNone())
            return true;

        i->onerror(mccmsg::to_errordscr(ptr->err));
        if (ptr->req->kind() == mccmsg::ReqKind::Dev)
            emit requestRemoved(bmcl::static_pointer_cast<const mccmsg::DevReq>(ptr->req));
        _requests.erase(id);
        return true;
    }
    case EventNote::EventId:
    {
        auto ptr = dynamic_cast<EventNote*>(event);
        if (!ptr)
            return QObject::event(event);
        NoteVisitorX visitor(this, ptr->note);
        ptr->note->visit(visitor);
        return true;
    }
    default:
        break;
    }

    return QObject::event(event);
}

const mccuav::ReqMap& CafService::requests() const
{
    return _requests;
}

void CafService::cancel(const mccmsg::RequestPtr& req)
{
    cancel(req->requestId());
}

void CafService::cancel(mccmsg::RequestId id)
{
    auto i = get(id);
    if (i.isNone() || i.unwrap().isCanceling())
        return;
    i.unwrap().cancel();
    caf::anon_send(_helper, bmcl::makeRc<const mccmsg::Cancel>(i.unwrap().req()));
}


