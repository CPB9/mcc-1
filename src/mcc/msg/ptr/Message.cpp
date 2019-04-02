#include <type_traits>
#include <bmcl/Logging.h>
#include <bmcl/StringView.h>
#include <fmt/format.h>
#include "mcc/msg/ptr/All.h"
#include "mcc/msg/ptr/NoteVisitor.h"
#include "mcc/msg/ptr/ReqVisitor.h"


namespace mccmsg {

Response::Response(const Request* request) : _request(request){}
Response::~Response() {}
RequestId Response::requestId() const { return _request->requestId(); }
const RequestPtr& Response::request() const { return _request; }

Cancel::Cancel(const RequestPtr& r) : _req(r) {}
Cancel::~Cancel() {}
RequestId Cancel::requestId()  const { return _req->requestId(); }
const RequestPtr Cancel::request() const { return _req; }

Request_State::Request_State(const Request* request, uint8_t progress) : _request(request), _progress(progress) {}
Request_State::Request_State(const Request* request, ReqResult result) : _request(request), _progress(100), _result(result) {}
Request_State::~Request_State() {}
const RequestPtr& Request_State::request() const { return _request; }
uint8_t Request_State::progress() const { return _progress; }
bmcl::Option<ReqResult> Request_State::result() const { return _result; }

Notification::Notification() {}
Notification::~Notification() {}

template <typename T>
ResponseImpl<T>::~ResponseImpl() {}
template <typename T>
ResponseImpl<T>::ResponseImpl(const Request* r) : Response(r) {} //временная мера. нужен конструктор на случай пустого item, т.е. для тега
template <typename T>
ResponseImpl<T>::ResponseImpl(const Request* r, const T& t) : Response(r), _data(t) {}
template <typename T>
ResponseImpl<T>::ResponseImpl(const Request* r, T&& t) : Response(r), _data(std::move(t)) {}
template <typename T>
const T& ResponseImpl<T>::data() const { return _data; }

template <typename T, typename Tag, typename R>
RequestImpl<T, Tag, R>::~RequestImpl() {}
template <typename T, typename Tag, typename R>
RequestImpl<T, Tag, R>::RequestImpl(const T& t) : _data(t) {}
template <typename T, typename Tag, typename R>
RequestImpl<T, Tag, R>::RequestImpl(T&& t) : _data(std::move(t)) {}
template <typename T, typename Tag, typename R>
const T& RequestImpl<T, Tag, R>::data() const { return _data; }
template <typename T, typename Tag, typename R>
void RequestImpl<T, Tag, R>::visit(ReqVisitor& v) const { v.visit(this); }

template <typename T>
NoteImpl<T>::~NoteImpl() {}
template <typename T>
NoteImpl<T>::NoteImpl(const T& t) : _data(t) {}
template <typename T>
NoteImpl<T>::NoteImpl(T&& t) : _data(std::move(t)) {}
template <typename T>
const T& NoteImpl<T>::data() const { return _data; }
template <typename T>
void NoteImpl<T>::visit(NoteVisitor& v) const { v.visit(this); }


#define MCCMSG_STANDART_OBJECTS(nmsp)                                           \
    template struct EventTmpl<nmsp::ObjName, EventKind::Registered>;            \
    template class NoteImpl<EventTmpl<nmsp::ObjName, EventKind::Registered>>;   \
    template class NoteImpl<nmsp::ObjDscr>;                                     \
                                                                                \
    template class ResponseImpl<nmsp::ObjDscr>;                                 \
    template class RequestImpl<nmsp::ObjDscr, nmsp::Register_Request_Tag, nmsp::ObjDscr>; \
    template class RequestImpl<nmsp::ObjName, nmsp::UnRegister_Request_Tag, nmsp::UnRegister_Response_Tag>; template class ResponseImpl<nmsp::UnRegister_Response_Tag>; \
    template class RequestImpl<nmsp::List_Request_Tag, nmsp::List_Request_Tag, nmsp::ObjNames>; template class ResponseImpl<nmsp::ObjNames>;                            \
    template class RequestImpl<nmsp::ObjName, nmsp::Description_Request_Tag, nmsp::ObjDscr>;                                                                            \
    template class RequestImpl<ProtocolValue, nmsp::DescriptionS_Request_Tag, nmsp::ObjDscr>;                                                                         \
    template class RequestImpl<nmsp::DescriptionList_Request_Tag, nmsp::DescriptionList_Request_Tag, nmsp::ObjDscrs>; template class ResponseImpl<nmsp::ObjDscrs>;      \
    template class RequestImpl<nmsp::Updater, nmsp::Update_Request_Tag, nmsp::ObjDscr>;

MCCMSG_STANDART_OBJECTS(channel);
MCCMSG_STANDART_OBJECTS(device);
MCCMSG_STANDART_OBJECTS(deviceUi);
MCCMSG_STANDART_OBJECTS(firmware);
MCCMSG_STANDART_OBJECTS(protocol);
MCCMSG_STANDART_OBJECTS(radar);
MCCMSG_STANDART_OBJECTS(tmSession);

template struct EventTmpl<channel::ObjName, EventKind::Activated>;
template class NoteImpl<EventTmpl<channel::ObjName, EventKind::Activated>>;

template struct EventTmpl<device::ObjName, EventKind::Activated>;
template class NoteImpl<EventTmpl<device::ObjName, EventKind::Activated>>;

template class NoteImpl<StatChannel>;
template class RequestImpl<EventTmpl<channel::ObjName, EventKind::Activated>, channel::Activate_Request_Tag, channel::Activate_Response_Tag>; template class ResponseImpl<channel::Activate_Response_Tag>;

template class NoteImpl<StatDevice>;
template class RequestImpl<EventTmpl<device::ObjName, EventKind::Activated>, device::Activate_Request_Tag, device::Activate_Response_Tag>; template class ResponseImpl<device::Activate_Response_Tag>;

template class NoteImpl<device::ConnectedObj>;
template class RequestImpl<device::ConnectObj, device::Connect_Request_Tag, device::Connect_Response_Tag>; template class ResponseImpl<device::Connect_Response_Tag>;

template class RequestImpl<tm::DumpObj, tm::Dump_Request_Tag, tm::Dump_Response_Tag>; template class ResponseImpl<mccmsg::tm::Dump_Response_Tag>;
template class NoteImpl<tm::LogObj>;
template class NoteImpl<tm::TmAnyPtr>;

template class RequestImpl<advanced::ChannelAndDeviceReqObj, advanced::ChannelAndDeviceRegister_Request_Tag, advanced::ChannelAndDeviceRepObj>; template class ResponseImpl<advanced::ChannelAndDeviceRepObj>;

ReqVisitor::ReqVisitor(const DefaultHandler& handler) : _handler(handler){}
ReqVisitor::~ReqVisitor(){}

#define REQ_VISIT(nmsp) \
    void ReqVisitor::visit(const nmsp::Register_Request* req) { _handler(req); }        \
    void ReqVisitor::visit(const nmsp::UnRegister_Request* req) { _handler(req); }      \
    void ReqVisitor::visit(const nmsp::List_Request* req) { _handler(req); }            \
    void ReqVisitor::visit(const nmsp::Description_Request* req) { _handler(req); }     \
    void ReqVisitor::visit(const nmsp::DescriptionS_Request* req) { _handler(req); }    \
    void ReqVisitor::visit(const nmsp::DescriptionList_Request* req) { _handler(req); } \
    void ReqVisitor::visit(const nmsp::Update_Request* req) { _handler(req); }


REQ_VISIT(channel)
REQ_VISIT(device)
REQ_VISIT(deviceUi)
REQ_VISIT(protocol)
REQ_VISIT(firmware)
REQ_VISIT(radar)
REQ_VISIT(tmSession)
void ReqVisitor::visit(const channel::Activate_Request* req) { _handler(req); }void ReqVisitor::visit(const device::Activate_Request* req) { _handler(req); }
void ReqVisitor::visit(const device::Connect_Request* req) { _handler(req); }
#undef REQ_VISIT

void ReqVisitor::visit(const tm::Dump_Request* req) { _handler(req); }
void ReqVisitor::visit(const advanced::ChannelAndDeviceRegister_Request* req) { _handler(req); }


NoteVisitor::NoteVisitor(const DefaultHandler& handler) : _handler(handler) {}
NoteVisitor::~NoteVisitor(){}

void NoteVisitor::visit(const tm::Item* note) { _handler(note); }
void NoteVisitor::visit(const tm::Log* note) { _handler(note); }
void NoteVisitor::visit(const channel::State* note) { _handler(note); }
void NoteVisitor::visit(const channel::Activated* note) { _handler(note); }
void NoteVisitor::visit(const device::State* note) { _handler(note); }
void NoteVisitor::visit(const device::Activated* note) { _handler(note); }
void NoteVisitor::visit(const device::Connected* note) { _handler(note); }

void NoteVisitor::visit(const channel::Registered* note) { _handler(note); }
void NoteVisitor::visit(const channel::Updated* note) { _handler(note); }

void NoteVisitor::visit(const device::Registered* note) { _handler(note); }
void NoteVisitor::visit(const device::Updated* note) { _handler(note); }

void NoteVisitor::visit(const deviceUi::Registered* note) { _handler(note); }
void NoteVisitor::visit(const deviceUi::Updated* note) { _handler(note); }

void NoteVisitor::visit(const firmware::Registered* note) { _handler(note); }
void NoteVisitor::visit(const firmware::Updated* note) { _handler(note); }

void NoteVisitor::visit(const protocol::Registered* note) { _handler(note); }
void NoteVisitor::visit(const protocol::Updated* note) { _handler(note); }

void NoteVisitor::visit(const radar::Registered* note) { _handler(note); }
void NoteVisitor::visit(const radar::Updated* note) { _handler(note); }

void NoteVisitor::visit(const tmSession::Registered* note) { _handler(note); }
void NoteVisitor::visit(const tmSession::Updated* note) { _handler(note); }

NotificationPtr makeTm(const TmAny* tm)
{
    return make<tm::Item>(mccmsg::TmAnyPtr(tm));
}

NotificationPtr makeNote(const Notification* note)
{
    return NotificationPtr(note);
}

MCC_MSG_DECLSPEC DbReqPtr makeReq(const DbReq* req)
{
    return DbReqPtr(req);
}

}
