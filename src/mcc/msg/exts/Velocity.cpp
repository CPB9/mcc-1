#include "mcc/msg/exts/Velocity.h"
#include <cmath>

namespace mccmsg {

TmVelocity::TmVelocity(const TmExtensionCounterPtr& counter) : ITmSimpleExtension(id(), info(), counter) {}
TmVelocity::~TmVelocity() {}
const TmExtension& TmVelocity::id() { static auto i = TmExtension::createOrNil("{26bc2979-fd81-4dae-a14e-10e07347c0de}"); return i; }
const char* TmVelocity::info() { return "velocity"; }
bmcl::Option<mccgeo::Position> TmVelocity::velocity() const { return _velocity; }
bmcl::Option<double> TmVelocity::speed() const { return _speed; }
void TmVelocity::set(bmcl::SystemTime t, const bmcl::Option<mccgeo::Position>& v)
{
    bool changed = (_velocity != v);
    _velocity = v;
    if (v.isNone())
        _speed.clear();

    if (changed)
        _speed = std::hypot(std::hypot(v->latitude(), v->longitude()), v->altitude());
    updated_(t, changed);
}

void TmVelocity::set(bmcl::SystemTime t, const bmcl::Option<double>& v)
{
    bool changed = (_speed != v);
    _speed = v;
    updated_(t, changed);
}
}
