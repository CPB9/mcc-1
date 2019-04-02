#include "mcc/msg/exts/Position.h"
#include <bmcl/Option.h>

namespace mccmsg {

TmPosition::TmPosition(const TmExtensionCounterPtr& counter) : ITmSimpleExtension(id(), info(), counter) {}
TmPosition::~TmPosition() {}
const TmExtension& TmPosition::id() { static auto i = TmExtension::createOrNil("{8bd11374-8c08-4ed7-9e36-bce641d9a188}"); return i; }
const char* TmPosition::info() { return "position"; }
bmcl::Option<mccgeo::Position> TmPosition::position() const { return _position; }
bmcl::Option<double> TmPosition::positionAccuracy() const { return _positionAccuracy; }
void TmPosition::set(bmcl::SystemTime t, const bmcl::Option<mccgeo::Position>& v, bmcl::Option<double> acc)
{
    bool changed = (_position != v || acc != _positionAccuracy);
    _position = v;
    _positionAccuracy = acc;
    updated_(t, changed);
}

}
