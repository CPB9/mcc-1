#include "mcc/vis/Ticks.h"

#include <cmath>
#include <cstdint>
#include <initializer_list>

namespace mccvis {

Ticks Ticks::fromMinMax(double min, double max)
{
    constexpr std::size_t minTicks = 5;
    constexpr std::size_t maxTicks = 15;

    static const std::initializer_list<double> steps = {1, 2, 2.5, 5, 10};
    double delta = std::abs(max - min);
    double scale = std::pow(10, std::round(std::log10(delta / maxTicks)));

    Ticks ticks;
    Ticks rv;
    for (double step : steps) {
        ticks.step = step * scale;
        ticks.min = std::floor(min / ticks.step) * ticks.step;
        double nticks = std::floor((max - ticks.min) / ticks.step);
        ticks.max = ticks.min + nticks * ticks.step;

        if (max * 0.995 > ticks.max) { //HACK
            ticks.max += ticks.step;
            nticks++;
        }
        //if (std::abs((std::abs(max - ticks.max) - ticks.step)) < (ticks.step / 100)) {
        //    ticks.max += ticks.step;
        //}
        if (nticks >= minTicks && nticks <= maxTicks) {
            return ticks;
        } else {
            rv = ticks;
        }
    }
    return rv;
}
}
