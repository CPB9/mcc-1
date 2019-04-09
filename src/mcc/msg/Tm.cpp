#include <bmcl/StringView.h>
#include "mcc/msg/FwdExt.h"
#include "mcc/msg/Tm.h"
#include "mcc/msg/Calibration.h"
#include "mcc/msg/File.h"
#include "mcc/msg/GroupState.h"
#include "mcc/msg/Route.h"
#include "mcc/msg/ParamList.h"
#include "mcc/msg/TmView.h"


namespace mccmsg {

TmVisitor::TmVisitor(const DefaultHandler& handler) : _handler(handler) {}
TmVisitor::~TmVisitor() {}
void TmVisitor::visit(const TmRoute& msg) { _handler((const TmAny*)&msg); };
void TmVisitor::visit(const TmRoutesList& msg) { _handler((const TmAny*)&msg); };
void TmVisitor::visit(const TmCalibration& msg) { _handler((const TmAny*)&msg); };
void TmVisitor::visit(const TmCommonCalibrationStatus& msg) { _handler((const TmAny*)&msg); };
void TmVisitor::visit(const TmGroupState& msg) { _handler((const TmAny*)&msg); };
void TmVisitor::visit(const ITmView& msg) { _handler((const TmAny*)&msg); };
void TmVisitor::visit(const ITmViewUpdate& msg) { _handler((const TmAny*)&msg); };
void TmVisitor::visit(const ITmPacketResponse& msg) { _handler((const TmAny*)&msg); };

TmAny::TmAny(const Device& device, bmcl::SystemTime time) : _device(device), _time(time) {}
TmAny::~TmAny() {}
const Device& TmAny::device() const { return _device; }
bmcl::SystemTime TmAny::time() const { return _time; }


void TmRoute::visit(TmVisitor* visitor) const { visitor->visit(*this); }
void TmRoutesList::visit(TmVisitor* visitor) const { visitor->visit(*this); }
void TmGroupState::visit(TmVisitor* visitor) const { visitor->visit(*this); }
void TmCommonCalibrationStatus::visit(TmVisitor* visitor) const { visitor->visit(*this); }
void TmCalibration::visit(TmVisitor* visitor) const { visitor->visit(*this); }
void ITmView::visit(TmVisitor* visitor) const { visitor->visit(*this); }
void ITmViewUpdate::visit(TmVisitor* visitor) const { visitor->visit(*this); }
void ITmPacketResponse::visit(TmVisitor* visitor) const { visitor->visit(*this); }

}
