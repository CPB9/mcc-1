#pragma once

#include "mcc/vis/Config.h"
#include "mcc/vis/Rc.h"
#include "mcc/vis/RadarParams.h"
#include "mcc/vis/Point.h"

#include <bmcl/Option.h>

#include <vector>

namespace mccvis {

class Slice;

struct RayEnd
{
    RayEnd()
        : hasTargetIntersections(false)
    {
    }

    RayEnd(Point peak, Point end, bool hasTargetIntersections)
        : peak(peak)
        , end(end)
        , hasTargetIntersections(hasTargetIntersections)
    {
    }

    Point peak;
    Point end;
    bool hasTargetIntersections;
};

struct Rays {
    Rays()
        : maxIsCut(false)
        , minIsCut(false)
    {
    }
    std::vector<RayEnd> ends;
    Point start;
    Point minEnd;
    Point maxEnd;
    Point minEdge;
    Point maxEdge;
    bool maxIsCut;
    bool minIsCut;
};

struct Hit {
    Hit(const Point& det, const Point& hit, double endX)
        : detection(det)
        , hit(hit)
        , endX(endX)
    {
    }

    Interval interval() const
    {
        if (hit.x() > endX) {
            return Interval(endX, hit.x());
        }
        return Interval(hit.x(), endX);
    }

    Point detection;
    Point hit;
    double endX;
};

class MCC_VIS_DECLSPEC Profile : public RefCountable {
public:
    struct Data {
        Point profile;
        Point target;
        double kview;
        double dy;
    };

    Profile(double direction, const PointVector& slice, const ViewParams& params);
    ~Profile();

    bmcl::Option<std::pair<double, double>> verticalVisionIntervalAt(double x) const;
    const std::vector<Data>& data() const;
    const Rays& rays() const;
    const IntervalVector& horizontalVisionIntervals() const;
    const IntervalVector& horizontalHitIntervals() const;
    const PointVector& viewIntersections() const;
    const PointVector& viewRegion() const;
    const std::vector<Hit>& hits() const;
    const std::vector<Hit>& outOfRangeHits() const;

    double hmin() const
    {
        return _hmin;
    }

    double hmax() const
    {
        return _hmax;
    }

    const Rect& rect() const
    {
        return _rect;
    }

    const Rect& rayRect() const
    {
        return _rayRect;
    }

    const ViewParams& params() const
    {
        return _params;
    }

    double viewAngle() const
    {
        return _viewAngle;
    }

    double peakDistance() const
    {
        return _peakDistance;
    }

    double minViewDistance() const
    {
        return _minViewDistance;
    }

    double maxViewDistance() const
    {
        return _maxViewDistance;
    }

    double minHitDistance() const
    {
        return _minHitDistance;
    }

    double maxHitDistance() const
    {
        return _maxHitDistance;
    }

    double direction() const
    {
        return _direction;
    }

    double totalRadarHeight() const
    {
        return _radarY;
    }

private:
    void findRays(const ViewParams& params, double radarY);

    template <bool isTowards>
    void findHits();

    bool findProfIntersection(std::vector<Data>::const_iterator it, Point* end);
    bool findProfAndAddTargetIntersection(std::vector<Data>::const_iterator it, Point* end, bool* hasTargetInt);

    template <bool isRelativeHeight, bool hasRefraction>
    void fillData(const PointVector& slice);

    void calcNextRocketInterval(const Point& p1, const Point& p2, Point* rocket);

    std::vector<Data> _data;
    Rays _rays;
    Rect _rect;
    Rect _rayRect;
    Point _start;
    ViewParams _params;
    IntervalVector _visionIntervals;
    IntervalVector _hitIntervals;
    PointVector _intersections;
    PointVector _viewRegion;
    std::vector<Hit> _hits;
    std::vector<Hit> _outOfRangeHits;
    double _hmin;
    double _hmax;
    double _radarY;
    double _viewAngle;
    double _peakDistance;
    double _minViewDistance;
    double _maxViewDistance;
    double _minHitDistance;
    double _maxHitDistance;
    double _direction;
};
}
