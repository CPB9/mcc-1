#pragma once

#include "mcc/Config.h"

#include "bmcl/FixedArrayView.h"

#include <QRgb>

#include <cstddef>
#include <cmath>

namespace mccui {

template <typename T>
struct Rgb {
public:
    constexpr Rgb(T r, T g, T b)
        : r(r)
        , g(g)
        , b(b)
    {
    }

    template <typename R>
    constexpr Rgb(const Rgb<R> rgb)
        : r(rgb.r)
        , g(rgb.g)
        , b(rgb.b)
    {
    }

    constexpr QRgb toQRgb() const
    {
        return qRgb(r, g, b);
    }

    T r;
    T g;
    T b;
};

template <typename T, typename S, std::size_t schemeSize>
class EqualDistanceGradient {
public:
    EqualDistanceGradient(T minValue, T maxValue, const std::array<Rgb<S>, schemeSize>& scheme)
        : _minValue(minValue)
        , _maxValue(maxValue)
        , _scheme(scheme)
    {
        static_assert(schemeSize >= 2, "invalid scheme size");
    }

    Rgb<T> calcColor(T value) const
    {
        if (value <= _minValue) {
            return _scheme[0];
        } else if (value >= _maxValue) {
            return _scheme[schemeSize - 1];
        }
        double loc = (schemeSize - 1) * double(value) / (_maxValue - _minValue);
        double integral;
        double frac = std::modf(loc, &integral);
        int i = (int)integral;
        return Rgb<T>(getChannel(_scheme[i].r, _scheme[i + 1].r, frac),
                      getChannel(_scheme[i].g, _scheme[i + 1].g, frac),
                      getChannel(_scheme[i].b, _scheme[i + 1].b, frac));
    }

private:
    inline T getChannel(S c1, S c2, double frac) const
    {
        return std::fma<double>(c2 - c1, frac, c1);
    }

    T _minValue;
    T _maxValue;
    std::array<Rgb<S>, schemeSize> _scheme;
};

using MapGradient = EqualDistanceGradient<double, uint8_t, 14>;

MCC_UI_DECLSPEC const MapGradient& getSpectralMapGradient();
}
