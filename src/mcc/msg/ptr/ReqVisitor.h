#pragma once
#include "mcc/Config.h"
#include <functional>
#include "mcc/msg/ptr/Fwd.h"

namespace mccmsg
{

#define  REQ_VISIT(nmsp)    \
    virtual void visit(const nmsp::Register_Request*);          \
    virtual void visit(const nmsp::UnRegister_Request*);        \
    virtual void visit(const nmsp::List_Request*);              \
    virtual void visit(const nmsp::Description_Request*);       \
    virtual void visit(const nmsp::DescriptionS_Request*);      \
    virtual void visit(const nmsp::DescriptionList_Request*);   \
    virtual void visit(const nmsp::Update_Request*);


class MCC_MSG_DECLSPEC ReqVisitor
{
public:
    using DefaultHandler = std::function<void(const DbReq*)>;
    ReqVisitor(const DefaultHandler& handler = [](const DbReq*) {});
    virtual ~ReqVisitor();

    REQ_VISIT(channel);
    REQ_VISIT(device);
    REQ_VISIT(deviceUi);
    REQ_VISIT(protocol);
    REQ_VISIT(firmware);
    REQ_VISIT(radar);
    REQ_VISIT(tmSession);
    virtual void visit(const channel::Activate_Request*);
    virtual void visit(const device::Activate_Request*);
    virtual void visit(const device::Connect_Request*);

    virtual void visit(const tm::Dump_Request*);
    virtual void visit(const advanced::ChannelAndDeviceRegister_Request*);

    MCC_DELETE_COPY_MOVE_CONSTRUCTORS(ReqVisitor);
protected:
    DefaultHandler _handler;
};

#undef REQ_VISIT

}
