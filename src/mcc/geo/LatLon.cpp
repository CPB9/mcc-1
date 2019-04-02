#include "mcc/geo/LatLon.h"

#include <bmcl/DoubleEq.h>

namespace mccgeo {

bool operator==(const LatLon& left, const LatLon& right)
{
    return bmcl::doubleEq(left.latitude(), right.latitude())
            && bmcl::doubleEq(left.longitude(), right.longitude());
}

bool operator!=(const LatLon &left, const LatLon &right)
{
    return !bmcl::doubleEq(left.latitude(), right.latitude()) ||
           !bmcl::doubleEq(left.longitude(), right.longitude());
}

const LatLon operator+(const LatLon &left, const LatLon &right)
{
    return LatLon(left._latitude + right._latitude, left._longitude + right._longitude);
}

const LatLon operator-(const LatLon &left, const LatLon &right)
{
    return LatLon(left._latitude - right._latitude, left._longitude - right._longitude);
}

}
