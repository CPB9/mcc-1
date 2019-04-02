#include <caf/actor_cast.hpp>
#include <caf/send.hpp>
#include <bmcl/StringView.h>
#include <bmcl/MakeRc.h>
#include "mcc/net/Cmd.h"
#include "mcc/net/Error.h"
#include "mcc/msg/Msg.h"
#include "mcc/msg/Cmd.h"

CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::Request_StatePtr)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::NotificationPtr)
CAF_ALLOW_UNSAFE_MESSAGE_TYPE(mccmsg::CmdRespAnyPtr)

namespace mccnet {

Cmd::Cmd(caf::response_promise&& pr, const mccmsg::DevReqPtr& cmd, mccmsg::Error e)
    : _pr(std::move(pr)), _cmd(cmd), _e(e)
{
}

Cmd::Cmd(caf::response_promise&& pr, const mccmsg::DevReqPtr& cmd, const caf::actor& core, mccmsg::Error e)
    : _pr(std::move(pr)), _cmd(cmd), _core(core), _e(e)
{
}

Cmd::Cmd(const mccmsg::DevReqPtr& cmd, mccmsg::Error e)
    : _cmd(cmd), _e(e)
{
}

Cmd::Cmd(const mccmsg::DevReqPtr& cmd, const caf::actor& core, mccmsg::Error e)
    : _cmd(cmd), _core(core), _e(e)
{
}

Cmd::~Cmd()
{
    sendFailed(_e);
}

void Cmd::sendDone(bmcl::Rc<const mccmsg::CmdRespAny>&& resp)
{
    if (_cmd.isNull())
        return;

    if (_core)
    {
        caf::anon_send(_core, bmcl::makeRc<const mccmsg::Request_State>(_cmd.get(), mccmsg::ReqResult::Done));
    }
    if (_pr.pending())
    {
        if (resp.isNull())
            resp = bmcl::makeRc<mccmsg::CmdRespEmpty>(_cmd);
        _pr.deliver(std::move(resp));
    }

    _cmd.reset();
}

void Cmd::sendFailed(caf::error&& e)
{
    if (_cmd.isNull())
        return;

    if (_core)
    {
        caf::anon_send(_core, bmcl::makeRc<const mccmsg::Request_State>(_cmd.get(), mccmsg::isMccCancel(e) ? mccmsg::ReqResult::Canceled : mccmsg::ReqResult::Failed));
    }
    if (_pr.pending())
        _pr.deliver(std::move(e));

    _cmd.reset();
}

void Cmd::sendFailed(const caf::error& e)
{
    auto tmp = e;
    sendFailed(std::move(tmp));
}

void Cmd::sendFailed(mccmsg::Error e)
{
    sendFailed(mccmsg::make_error(e));
}

void Cmd::sendFailed(bmcl::StringView text, mccmsg::Error e)
{
    sendFailed(mccmsg::make_error(e, text));
}

void Cmd::sendCanceled()
{
    sendFailed(mccmsg::make_error(mccmsg::Error::Canceled));
}

void Cmd::sendProgress(uint8_t progress)
{
    if (_cmd.isNull())
        return;

    if (_pr.pending())
    {
        auto a = caf::actor_cast<caf::actor>(_pr.source());
        caf::anon_send(a, bmcl::makeRc<const mccmsg::Request_State>(_cmd.get(), progress));
    }
    if (_core)
    {
        caf::anon_send(_core, bmcl::makeRc<const mccmsg::Request_State>(_cmd.get(), progress));
    }
}

void Cmd::sendProgress(std::size_t part, std::size_t whole, uint8_t shift, uint8_t limit)
{
    if (whole == 0) whole = 1;

    uint8_t progress = shift + (limit - shift) * part / whole;
    if (progress == 100 && part != whole)
        progress = 99;
    sendProgress(progress);
}

const mccmsg::DevReqPtr& Cmd::item() const
{
    return _cmd;
}

bool Cmd::isValid() const
{
    return !_cmd.isNull();
}

bmcl::Option<mccmsg::RequestId> Cmd::requestId() const
{
    if (_cmd.isNull())
        return bmcl::None;
    return _cmd->requestId();
}


}
