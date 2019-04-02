#pragma once
#include "mcc/Config.h"
#include <string>
#include "mcc/Rc.h"
#include "mcc/msg/Msg.h"
#include "mcc/msg/ptr/Fwd.h"

#include <bmcl/TimeUtils.h>

#define MSG_ALLOW_CAF(nmsp, name) CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::nmsp::name ## _RequestPtr); CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::nmsp::name ## _ResponsePtr)

namespace mccmsg {

class ReqVisitor;
class NoteVisitor;

class MCC_MSG_DECLSPEC Notification : public Message
{
public:
    ~Notification();
    virtual void visit(NoteVisitor& v) const = 0;
protected:
    Notification();
};

template <typename T>
class MCC_MSG_DECLSPEC ResponseImpl : public Response
{
public:
    ~ResponseImpl();
    ResponseImpl(const Request* r);
    ResponseImpl(const Request* r, const T& t);
    ResponseImpl(const Request* r, T&& t);
    template<typename... A>
    ResponseImpl(const Request* r, A&&... args) : Response(r), _data(std::forward<A>(args)...) {}

    const T& data() const;
private:
    T _data;
};

template <typename T, typename Tag, typename R>
class MCC_MSG_DECLSPEC RequestImpl : public DbReq
{
public:
    using RequestPtr = bmcl::Rc<const RequestImpl>;
    using Response = ResponseImpl<R>;
    using ResponsePtr = bmcl::Rc<const ResponseImpl<R>>;
    ~RequestImpl();
    RequestImpl(const T& t);
    RequestImpl(T&& t);
    template<typename... A>
    RequestImpl(A&&... args) : _data(std::forward<A>(args)...) {}

    const T& data() const;
    void visit(ReqVisitor& v) const override;
private:
    T _data;
};

template <typename T>
class MCC_MSG_DECLSPEC NoteImpl : public Notification
{
public:
    ~NoteImpl();
    NoteImpl(const T& t);
    NoteImpl(T&& t);
    template<typename... A>
    NoteImpl(A&&... args) : _data(std::forward<A>(args)...) {}

    const T& data() const;
    void visit(NoteVisitor& v) const override;
private:
    T _data;
};

template<typename T, typename... A>
bmcl::Rc<const T> make(A&&... args)
{
    static_assert(std::is_base_of<Message, T>::value, "only messages can be passed on");
    return bmcl::Rc<const T>(new T(std::forward<A>(args)...));
}

MCC_MSG_DECLSPEC NotificationPtr makeNote(const Notification* note);
MCC_MSG_DECLSPEC DbReqPtr makeReq(const DbReq* req);

}
