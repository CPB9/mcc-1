#pragma once

#include "mcc/vis/Config.h"

namespace mccvis {

struct MCC_VIS_DECLSPEC Ticks {
    double min;
    double max;
    double step;

    static Ticks fromMinMax(double min, double max);
};
}
