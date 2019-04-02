#include "mcc/msg/exts/Attitude.h"
#include <bmcl/Option.h>

namespace mccmsg {

TmAttitude::TmAttitude(const TmExtensionCounterPtr& counter) : ITmSimpleExtension(id(), info(), counter) {}
TmAttitude::~TmAttitude() {}
const TmExtension& TmAttitude::id() { static auto i = TmExtension::createOrNil("{1c82028e-1f0e-4a85-b967-2d0641bcc9ea}"); return i; }
const char* TmAttitude::info() { return "attitude"; }
bmcl::Option<mccgeo::Attitude> TmAttitude::attitude() const { return _attitude; }
void TmAttitude::set(bmcl::SystemTime t, const bmcl::Option<mccgeo::Attitude>& v)
{
    bool changed = (_attitude != v);
    _attitude = v;
    updated_(t, changed);
}

}
