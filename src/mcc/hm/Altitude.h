#pragma once

#include "mcc/hm/Config.h"

#include <bmcl/DefaultOption.h>

#include <cstdint>
#include <cmath>

namespace mcchm {

struct AltDescriptor {
    static constexpr double defaultValue()
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    static inline bool isDefault(double value)
    {
        return std::isnan(value);
    }
};

using Altitude = bmcl::DefaultOption<double, AltDescriptor>;

constexpr std::int16_t voidSrtmAltitude = -32768;

using SrtmAltDescriptor = bmcl::DefaultOptionDescriptor<std::int16_t, voidSrtmAltitude>;

using SrtmAltitude = bmcl::DefaultOption<std::int16_t, SrtmAltDescriptor>;
}
