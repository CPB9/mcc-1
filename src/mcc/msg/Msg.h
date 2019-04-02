#pragma once
#include "mcc/Config.h"
#include "mcc/Rc.h"
#include "mcc/msg/Fwd.h"
#include "mcc/msg/Objects.h"
#include <bmcl/TimeUtils.h>
#include <bmcl/Either.h>
#include <bmcl/StringView.h>
#include <bmcl/Option.h>

namespace mccmsg {

class ReqVisitor;
class NoteVisitor;

class MCC_MSG_DECLSPEC Message : public mcc::RefCountable
{
public:
    MCC_DELETE_COPY_MOVE_CONSTRUCTORS(Message);
    virtual ~Message();
protected:
    Message();
private:
};

enum class ReqKind : uint8_t
{
    Db,
    Dev
};

enum class ReqResult : uint8_t
{
    Done,
    Failed,
    Canceled,
};

class MCC_MSG_DECLSPEC Request : public Message
{
public:
    RequestId requestId() const;
    ReqKind kind() const;
    virtual const char* nameXXX() const = 0;
    virtual const char* info() const = 0;
protected:
    ~Request() override;
    Request(ReqKind);
private:
    RequestId _requestId;
    ReqKind _kind;
};
using RequestPtr = bmcl::Rc<const Request>;

class MCC_MSG_DECLSPEC Cancel : public Message
{
public:
    explicit Cancel(const RequestPtr& request);
    ~Cancel();
    RequestId requestId() const;
    const RequestPtr request() const;
private:
    RequestPtr _req;
};
using CancelPtr = bmcl::Rc<const Cancel>;

class MCC_MSG_DECLSPEC DevReq : public Request
{
public:
    using RequestPtr = bmcl::Rc<const DevReq>;
    using Response = CmdRespAny;
    ~DevReq() override;
    DevReq(const DeviceOrGroup& to, bmcl::StringView trait, bmcl::StringView cmd);
    virtual void visit(CmdVisitor* visitor) const = 0;

    bmcl::Option<Group> group() const;
    bmcl::Option<Device> device() const;

    const std::string& trait()   const;
    const std::string& command() const;
    std::string        short_name() const;
    std::string        name()    const;
private:
    DeviceOrGroup _to;

    //deprecated
    std::string _trait;
    std::string _command;
};
using DevReqPtr = bmcl::Rc<const DevReq>;

class MCC_MSG_DECLSPEC DbReq : public Request
{
public:
    ~DbReq() override;
    DbReq();
    virtual void visit(ReqVisitor& visitor) const = 0;
    const char* nameXXX() const override;
    const char* info() const override;
private:
};
using DbReqPtr = bmcl::Rc<const DbReq>;

class MCC_MSG_DECLSPEC Response : public Message
{
public:
    RequestId requestId() const;
    const RequestPtr& request() const;
protected:
    explicit Response(const Request* request);
    ~Response();
private:
    RequestPtr _request;
};

class MCC_MSG_DECLSPEC Request_State : public Message
{
public:
    Request_State(const Request* request, uint8_t progress);
    Request_State(const Request* request, ReqResult result);
    ~Request_State();
    const RequestPtr& request() const;
    uint8_t progress() const;
    bmcl::Option<ReqResult> result() const;
private:
    RequestPtr _request;
    uint8_t    _progress;
    bmcl::Option<ReqResult> _result;
};
using Request_StatePtr = bmcl::Rc<const Request_State>;

MCC_MSG_DECLSPEC DevReqPtr makeCmd(const DevReq*);

}
