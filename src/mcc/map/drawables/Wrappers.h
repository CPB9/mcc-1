#pragma once

#include "mcc/map/Fwd.h"

#include <bmcl/Math.h>

#include <cmath>
#include <utility>

namespace mccmap {

template <typename B>
class EmptyWrapper : public B {
public:
    template <typename... A>
	EmptyWrapper(bmcl::InPlaceType, A&&... args)
        : B(std::forward<A>(args)...)
    {
    }

    void reset(B&& base)
    {
        B::operator=(std::forward<B>(base));
    }

    void update(const EmptyWrapper<B>& next, const MapRect* rect)
    {
        (void)next;
        (void)rect;
    }

    void setSingle()
    {
    }
};

template <typename B>
class AngleWrapper : public B {
public:
    template <typename... A>
    AngleWrapper(bmcl::InPlaceType, A&&... args)
		: B(bmcl::InPlace, std::forward<A>(args)...)
        , _angle(90)
    {
    }

    void setAngle(double angle)
    {
        _angle = angle;
    }

    double angle() const
    {
        return _angle;
    }

    void update(const AngleWrapper<B>& next, const MapRect* rect)
    {
        B::update(next, rect);
        const QPointF& p1 = B::position();
        const QPointF& p2 = next.position();
        _angle = bmcl::radiansToDegrees(std::atan2(-p2.y() + p1.y(), p2.x() - p1.x()));
    }

    void setSingle()
    {
        B::setSingle();
        _angle = 0;
    }

private:
    double _angle;
};

template <typename B>
class DistanceWrapper : public B {
public:
    template <typename... A>
    DistanceWrapper(bmcl::InPlaceType, A&&... args)
		: B(bmcl::InPlace, std::forward<A>(args)...)
        , _distance(0)
    {
    }

    void setDistance(double distance)
    {
        _distance = distance;
    }

    double distance() const
    {
        return _distance;
    }

    void update(const DistanceWrapper<B>& next, const MapRect* rect)
    {
        B::update(next, rect);
        mccgeo::LatLon curLatLon = B::toLatLon(rect);
        mccgeo::LatLon nextLatLon = next.toLatLon(rect);
        _distance = rect->projection().calcDistance(curLatLon, nextLatLon);
    }

    void setSingle()
    {
        B::setSingle();
        _distance = 0;
    }

private:
    double _distance;
};
}
