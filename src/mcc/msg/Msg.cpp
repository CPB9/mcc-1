#include <bmcl/Option.h>
#include <bmcl/MakeRc.h>
#include "mcc/msg/Msg.h"

namespace mccmsg {

std::atomic<RequestId> COUNTER(0);

Message::Message()  {}
Message::~Message() {}

Request::Request(ReqKind k) : _requestId(COUNTER.fetch_add(1)), _kind(k) {}
Request::~Request() {}
RequestId Request::requestId() const { return _requestId; }
ReqKind Request::kind() const { return _kind; }

DevReq::~DevReq() {}
DevReq::DevReq(const DeviceOrGroup& to, bmcl::StringView trait, bmcl::StringView cmd) : Request(ReqKind::Dev), _to(to), _command(cmd.toStdString()), _trait(trait.toStdString()){}
bmcl::Option<mccmsg::Group> DevReq::group() const { return _to.group(); }
bmcl::Option<Device> DevReq::device() const { return _to.device(); }

//deprecated
const std::string& DevReq::trait()   const { return _trait; }
const std::string& DevReq::command() const { return _command; }
std::string        DevReq::short_name() const { return _trait + "." + _command; }
std::string        DevReq::name()    const
{
    if (_to.device().isSome())
        return _to.device()->toStdString() + "." + _trait + "." + _command;
    else
        return _to.group()->toStdString() + "." + _trait + "." + _command;
}


DbReq::~DbReq() {}
DbReq::DbReq() : Request(ReqKind::Db) {}
const char* DbReq::nameXXX() const { return "dbreq"; }
const char* DbReq::info() const { return "dbreq"; }

DevReqPtr makeCmd(const DevReq* r)
{
    return bmcl::wrapRc(r);
}

}
