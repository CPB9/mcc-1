#pragma once

#include "mcc/Config.h"

#include "bmcl/FixedArrayView.h"

#include <QRgb>

#include <type_traits>
#include <cstddef>
#include <cmath>
#include <cassert>

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

using Rgb8 = Rgb<std::uint8_t>;
using Rgbf = Rgb<float>;
using Rgbd = Rgb<double>;

template <typename T, typename S, std::size_t schemeSize, typename F>
class EqualDistanceGradient {
public:
    EqualDistanceGradient(T minValue, T maxValue, const std::array<Rgb<S>, schemeSize>& scheme)
        : _minValue(minValue)
        , _maxValue(maxValue)
        , _scheme(scheme)
    {
        assert(_maxValue > _minValue);
        static_assert(std::is_floating_point<F>::value, "F must be floating point");
        static_assert(schemeSize >= 2, "invalid scheme size");
    }

    Rgb<T> calcColor(T value) const
    {
        if (value <= _minValue) {
            return _scheme[0];
        } else if (value >= _maxValue) {
            return _scheme[schemeSize - 1];
        }
        F loc = (schemeSize - 1) * F(value) / (_maxValue - _minValue);
        F integral;
        F frac = std::modf(loc, &integral);
        std::size_t i = integral;
        const Rgb<S>& s1 = _scheme[i];
        const Rgb<S>& s2 = _scheme[i + 1];
        return Rgb<T>(getChannel(s1.r, s2.r, frac),
                      getChannel(s1.g, s2.g, frac),
                      getChannel(s1.b, s2.b, frac));
    }

private:
    inline T getChannel(S c1, S c2, F frac) const
    {
        return std::fma<F>(c2 - c1, frac, c1);
    }

    T _minValue;
    T _maxValue;
    std::array<Rgb<S>, schemeSize> _scheme;
};

using MapGradient = EqualDistanceGradient<float, std::uint8_t, 12, float>;

MCC_UI_DECLSPEC const MapGradient& getSpectralMapGradient();
}
