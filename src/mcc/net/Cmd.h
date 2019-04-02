#pragma once
#include "mcc/Config.h"
#include <string>
#include <caf/response_promise.hpp>

#include "mcc/Rc.h"
#include "mcc/msg/ptr/Fwd.h"
#include "mcc/net/Error.h"

namespace mccnet {

class MCC_PLUGIN_NET_DECLSPEC Cmd : public mcc::RefCountable
{
public:
    Cmd() = delete;
    Cmd(const Cmd&) = delete;
    Cmd(Cmd&&) = delete;

    Cmd(caf::response_promise&& pr, const mccmsg::DevReqPtr& cmd, mccmsg::Error e = mccmsg::Error::CmdUnknown);
    Cmd(caf::response_promise&& pr, const mccmsg::DevReqPtr& cmd, const caf::actor& core, mccmsg::Error e = mccmsg::Error::CmdUnknown);
    Cmd(const mccmsg::DevReqPtr& cmd, mccmsg::Error e = mccmsg::Error::CmdUnknown);
    Cmd(const mccmsg::DevReqPtr& cmd, const caf::actor& core, mccmsg::Error e = mccmsg::Error::CmdUnknown);

    ~Cmd();
    bool isValid() const;
    bmcl::Option<mccmsg::RequestId> requestId() const;

    const mccmsg::DevReqPtr& item() const;
    void sendDone(bmcl::Rc<const mccmsg::CmdRespAny>&& = nullptr);
    void sendCanceled();
    void sendProgress(uint8_t progress);
    void sendProgress(std::size_t part, std::size_t whole, uint8_t shift = 0, uint8_t limit = 100); // shift + (limit-shift)*part/whole
    void sendFailed(caf::error&& e);
    void sendFailed(const caf::error& e);
    void sendFailed(mccmsg::Error e);
    void sendFailed(bmcl::StringView text, mccmsg::Error e = mccmsg::Error::CmdFailed);

private:
    mccmsg::Error _e;
    mccmsg::DevReqPtr _cmd;

    caf::response_promise _pr;
    caf::actor _core;
};
using CmdPtr = mcc::Rc<Cmd>;

}
