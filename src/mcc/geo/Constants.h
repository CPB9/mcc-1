#pragma once

#include "mcc/geo/Config.h"
#include "bmcl/Math.h"

namespace mccgeo {

#define DEFINE_FLATTENING(name, value, divisor)      \
template <typename T>                                \
constexpr inline T name();                           \
                                                     \
template <>                                          \
constexpr inline float name()                        \
{                                                    \
    return 1.0 / (float(value) / divisor);           \
}                                                    \
                                                     \
template <>                                          \
constexpr inline double name()                       \
{                                                    \
    return 1.0 / (double(value) / divisor);          \
}                                                    \
                                                     \
template <>                                          \
constexpr inline long double name()                  \
{                                                    \
    return 1.0 / (((long double)(value)) / divisor); \
}

BMCL_DEFINE_MATH_CONSTANT(wgs84a, 6378137.0);

DEFINE_FLATTENING(wgs84f, 298257223563LL, 1000000000);

#undef DEFINE_FLATTENING
}
