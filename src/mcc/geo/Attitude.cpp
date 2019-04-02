#include "mcc/geo/Attitude.h"

#include <bmcl/DoubleEq.h>

namespace mccgeo {

bool Attitude::operator==(const Attitude& other) const
{
    return bmcl::doubleEq(_heading, other._heading)
        && bmcl::doubleEq(_pitch, other._pitch)
        && bmcl::doubleEq(_roll, other._roll);
}

bool Attitude::operator!=(const Attitude& other) const
{
    return !(*this == other);
}
}
