#pragma once
#include "mcc/Config.h"
#include <functional>
#include "mcc/msg/ptr/Fwd.h"

namespace mccmsg
{

class MCC_MSG_DECLSPEC NoteVisitor
{
public:
    using DefaultHandler = std::function<void(const Notification*)>;
    NoteVisitor(const DefaultHandler& handler = [](const Notification*) {});
    virtual ~NoteVisitor();

    virtual void visit(const tm::Item*);
    virtual void visit(const tm::Log*);
    virtual void visit(const channel::State*);
    virtual void visit(const channel::Activated*);
    virtual void visit(const device::State*);
    virtual void visit(const device::Activated*);
    virtual void visit(const device::Connected*);

    virtual void visit(const channel::Registered*);
    virtual void visit(const channel::Updated*);

    virtual void visit(const device::Registered*);
    virtual void visit(const device::Updated*);

    virtual void visit(const deviceUi::Registered*);
    virtual void visit(const deviceUi::Updated*);

    virtual void visit(const firmware::Registered*);
    virtual void visit(const firmware::Updated*);

    virtual void visit(const protocol::Registered*);
    virtual void visit(const protocol::Updated*);

    virtual void visit(const radar::Registered*);
    virtual void visit(const radar::Updated*);

    virtual void visit(const tmSession::Registered*);
    virtual void visit(const tmSession::Updated*);

    MCC_DELETE_COPY_MOVE_CONSTRUCTORS(NoteVisitor);
protected:
    DefaultHandler _handler;
};

}
