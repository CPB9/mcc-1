#include "mcc/ui/Trackable.h"
#include "mcc/geo/LatLon.h"

#include <bmcl/Option.h>

namespace mccui {

bmcl::Option<mccgeo::LatLon> Trackable::position() const
{
    return bmcl::None;
}
}
