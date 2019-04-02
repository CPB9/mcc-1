#pragma once
#include "mcc/Config.h"
#include <functional>
#include <bmcl/TimeUtils.h>
#include "mcc/Rc.h"
#include "mcc/msg/Fwd.h"
#include "mcc/msg/Msg.h"
#include "mcc/msg/Objects.h"

namespace mccmsg {

class MCC_MSG_DECLSPEC TmVisitor {
public:
    using DefaultHandler = std::function<void(const TmAny*)>;
    TmVisitor(const DefaultHandler& handler = [](const TmAny*) {});
    virtual ~TmVisitor();
    virtual void visit(const TmMotion& msg);
    virtual void visit(const TmRoute& msg);
    virtual void visit(const TmRoutesList& msg);
    virtual void visit(const TmCalibration& msg);
    virtual void visit(const TmCommonCalibrationStatus& msg);
    virtual void visit(const TmGroupState& msg);
    virtual void visit(const ITmView& msg);
    virtual void visit(const ITmViewUpdate& msg);
    virtual void visit(const ITmPacketResponse& msg);
private:
    DefaultHandler _handler;
};

class MCC_MSG_DECLSPEC TmAny : public mcc::RefCountable
{
public:
    TmAny(const Device& device, bmcl::SystemTime time = bmcl::SystemClock::now());
    virtual ~TmAny();
    virtual void visit(TmVisitor* visitor) const = 0;
    const Device& device() const;
    bmcl::SystemTime time() const;
private:
    Device _device;
    bmcl::SystemTime _time;
};

class MCC_MSG_DECLSPEC CmdGetTmView : public DevReq
{
public:
    CmdGetTmView(const Device& device);
    ~CmdGetTmView();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
private:
};

}
