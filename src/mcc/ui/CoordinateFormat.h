#pragma once

#include "mcc/Config.h"

#include <bmcl/Either.h>

namespace mccui {

enum class AngularFormat {
    Degrees = 0,
    DegreesMinutes,
    DegreesMinutesSeconds,
};

class MCC_UI_DECLSPEC CoordinateFormat {
public:
    explicit CoordinateFormat(const char* units);
    explicit CoordinateFormat(AngularFormat fmt);

    bool isAngular() const;
    bool isLinear() const;

    AngularFormat unwrapAngular() const;
    const char* unwrapLinear() const;

    void setLinear(const char* units);
    void setAngular(AngularFormat fmt);

private:
    bmcl::Either<AngularFormat, const char*> _data;
};
}
