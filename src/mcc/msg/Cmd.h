#pragma once
#include "mcc/Config.h"
#include <string>
#include <vector>
#include <functional>
#include <bmcl/StringView.h>
#include <bmcl/TimeUtils.h>
#include "mcc/Rc.h"
#include "mcc/msg/NetVariant.h"
#include "mcc/msg/Fwd.h"
#include "mcc/msg/Msg.h"
#include "mcc/msg/Objects.h"

namespace mccmsg {

using Devices = std::vector<Device>;
using CmdParams = std::vector<NetVariant>;

class MCC_MSG_DECLSPEC CmdVisitor
{
public:
    using DefaultHandler = std::function<void(const DevReq*)>;
    CmdVisitor(const DefaultHandler& handler = [](const DevReq*) {});
    virtual ~CmdVisitor();
    virtual void visit(const CmdParamList* msg);
    virtual void visit(const CmdRouteSet* msg);
    virtual void visit(const CmdRouteSetNoActive* msg);
    virtual void visit(const CmdRouteSetActive* msg);
    virtual void visit(const CmdRouteSetActivePoint* msg);
    virtual void visit(const CmdRouteSetDirection* msg);
    virtual void visit(const CmdRouteGet* msg);
    virtual void visit(const CmdRouteGetList* msg);
    virtual void visit(const CmdRouteCreate* msg);
    virtual void visit(const CmdRouteRemove* msg);
    virtual void visit(const CmdRouteClear* msg);
    virtual void visit(const CmdGetFrm* msg);
    virtual void visit(const CmdFileGetList* msg);
    virtual void visit(const CmdFileUpload* msg);
    virtual void visit(const CmdFileDownload* msg);
    virtual void visit(const CmdCalibrationStart* msg);
    virtual void visit(const CmdCalibrationCancel* msg);
    virtual void visit(const CmdParamRead* msg);
    virtual void visit(const CmdParamWrite* msg);
    virtual void visit(const CmdGroupNew* msg);
    virtual void visit(const CmdGroupDelete* msg);
    virtual void visit(const CmdGroupAttach* msg);
    virtual void visit(const CmdGroupDetach* msg);
    virtual void visit(const CmdGroupSwitch* msg);
    virtual void visit(const CmdPacketRequest* msg);
    virtual void visit(const CmdGetTmView* msg);
protected:
    DefaultHandler _handler;
};

class MCC_MSG_DECLSPEC CmdRespVisitor
{
public:
    CmdRespVisitor();
    virtual ~CmdRespVisitor();
    virtual void visit(const CmdFileGetListResp* msg);
    virtual void visit(const CmdRespEmpty* msg);
};

class MCC_MSG_DECLSPEC CmdRespAny : public Response
{
public:
    CmdRespAny(const DevReq* cmd);
    CmdRespAny(const DevReqPtr& cmd);
    virtual ~CmdRespAny();
    virtual void visit(CmdRespVisitor* visitor) const = 0;
private:
};

class MCC_MSG_DECLSPEC CmdRespEmpty : public CmdRespAny
{
public:
    CmdRespEmpty(const DevReq* cmd);
    CmdRespEmpty(const DevReqPtr& cmd);
    virtual ~CmdRespEmpty();
    void visit(CmdRespVisitor* visitor) const override;
private:
};

}
