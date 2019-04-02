#include "mcc/msg/exts/ControlOwner.h"
#include <cmath>

namespace mccmsg {

TmControlOwner::TmControlOwner(const TmExtensionCounterPtr& counter) : ITmSimpleExtension(id(), info(), counter) {}
TmControlOwner::~TmControlOwner() {}
const TmExtension& TmControlOwner::id() { static auto i = TmExtension::createOrNil("{bc4978ac-e761-4345-be17-1e8290730177}"); return i; }
const char* TmControlOwner::info() { return "control owner"; }
bmcl::Option<ControlOwner> TmControlOwner::owner() const { return _owner; }
void TmControlOwner::set(bmcl::SystemTime t, const bmcl::Option<ControlOwner>& v)
{
    bool changed = (_owner != v);
    _owner = v;
    updated_(t, changed);
}

}
