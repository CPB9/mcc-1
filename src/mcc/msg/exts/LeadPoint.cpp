#include "mcc/msg/exts/LeadPoint.h"

namespace mccmsg {

TmLeadPoint::TmLeadPoint(const TmExtensionCounterPtr& counter) : ITmSimpleExtension(id(), info(), counter) {}
TmLeadPoint::~TmLeadPoint() {}
const TmExtension& TmLeadPoint::id() { static auto i = TmExtension::createOrNil("{355443c7-d233-4ac6-99f9-b5525f5935ba}"); return i; }
const char* TmLeadPoint::info() { return "lead point"; }
const bmcl::Option<mccgeo::Position>& TmLeadPoint::position() const { return _leadPoint; }
void TmLeadPoint::set(bmcl::SystemTime t, const bmcl::Option<mccgeo::Position>& v)
{
    bool changed = (_leadPoint != v);
    _leadPoint = v;
    updated_(t, changed);
}

}
