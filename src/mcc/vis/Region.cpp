#include "mcc/vis/Region.h"

#include <bmcl/Option.h>
#include <bmcl/Logging.h>
#include <bmcl/Math.h>
#include <bmcl/OptionSize.h>

#include <algorithm>
#include <cassert>
#include <set>
#include <unordered_map>
#include <iostream>
#include <QDebug>
#include <cmath>
#include <deque>

namespace mccvis {

inline double calcArea(const Interval& i, double angleStep)
{
    double a = i.start();
    double b = i.end();
    return (b * b - a * a) * bmcl::pi<double>() * angleStep / 360.0;
}

static inline double circleArea(double r, double angle)
{
    return bmcl::pi<double>() * r * r * (angle / 360.0);
}

struct RegionBuilder {
    explicit RegionBuilder(double angleStep, bool isSegment)
        : dirIndex(0)
        , angleStep(angleStep)
        , isSegment(isSegment)
    {
    }

    std::vector<Directed<IntervalVector>> allIntervals;
    std::deque<Directed<Interval>> currentIntervalPolygon;
    std::vector<PointVector> finishedPolygons;
    std::size_t dirIndex;
    double angleStep;
    bool isSegment;

    bool skipToFirstNonEmpty()
    {
        std::size_t j = dirIndex;
        for (; dirIndex < allIntervals.size(); dirIndex++) {
            if (!allIntervals[dirIndex].value.empty()) {
                return true;
            }
        }

        for (dirIndex = 0; dirIndex < j; dirIndex++) {
            if (!allIntervals[dirIndex].value.empty()) {
                return true;
            }
        }

        return false;
    }

    void finishCurrent()
    {
        PointVector polygon;
        polygon.reserve(6 * currentIntervalPolygon.size());
        double delta = angleStep / 2.0 * 1.001;

        for (const auto& c : currentIntervalPolygon) {
            double val = c.value.end();
            polygon.emplace_back(c.direction - delta, val);
            polygon.emplace_back(c.direction, val);
            polygon.emplace_back(c.direction + delta, val);
        }

        for (auto it = currentIntervalPolygon.rbegin(); it < currentIntervalPolygon.rend(); it++) {
            double val = it->value.start();
            polygon.emplace_back(it->direction + delta, val);
            polygon.emplace_back(it->direction, val);
            polygon.emplace_back(it->direction - delta, val);
        }

        finishedPolygons.emplace_back(std::move(polygon));

        currentIntervalPolygon.clear();
    }

    template <bool isForward>
    void findAllDirection(std::size_t startDirIndex, Interval* int1)
    {
        while (true) {
            if (isForward) {
                if (dirIndex != (allIntervals.size() - 1)) {
                    dirIndex = dirIndex + 1;
                } else {
                    if (isSegment) {
                        return;
                    }
                    dirIndex = 0;
                }
            } else {
                if (dirIndex != 0) {
                    dirIndex = dirIndex - 1;
                } else {
                    if (isSegment) {
                        return;
                    }
                    dirIndex = allIntervals.size() - 1;
                }
            }
            if (dirIndex == startDirIndex) {
                return;
            }
            auto& dir = allIntervals[dirIndex];
            if (dir.value.empty()) {
                return;
            }
            bmcl::OptionSize foundIndex;
            double maxIntersection = 0;
            for (std::size_t j = 0; j < dir.value.size(); j++) {
                std::size_t currentIndex = dir.value.size() - j - 1;
                const Interval& int2 = dir.value[currentIndex];
                if (int2.start() >= int1->end() || int1->start() >= int2.end()) {
                    //no intersection
                } else {
                    double intersection = std::min(int1->end(), int2.end()) - std::max(int1->start(), int2.start());
                    if (intersection > maxIntersection) {
                        foundIndex = currentIndex;
                        maxIntersection = intersection;
                    }
                }
            }
            if (foundIndex.isSome()) {
                *int1 = dir.value[foundIndex.unwrap()];
                if (isForward) {
                    currentIntervalPolygon.emplace_back(dir.direction, *int1);
                } else {
                    currentIntervalPolygon.emplace_front(dir.direction, *int1);
                }
                dir.value.erase(dir.value.begin() + foundIndex.unwrap());
            } else {
                return;
            }
        }
    }

    bool calcNext()
    {
        if (!skipToFirstNonEmpty()) {
            return false;
        }

        std::size_t start = dirIndex;

        Interval int1 = allIntervals[dirIndex].value.back();
        Interval intStart = int1;

        currentIntervalPolygon.emplace_back(allIntervals[dirIndex].direction, allIntervals[dirIndex].value.back());
        allIntervals[dirIndex].value.pop_back();

        findAllDirection<true>(start, &int1);

        dirIndex = start;
        int1 = intStart;

        findAllDirection<false>(start, &int1);

        finishCurrent();
        return true;
    }

    std::vector<PointVector> calcAll()
    {
        while (calcNext()) {}

        return std::move(finishedPolygons);
    }
};

Region::Region(const std::vector<Rc<Profile>>& slices, const ViewParams& params)
{
    _profiles = slices;
    _params = params;
    _maxDistance = 0;

    updateData();
}

Region::Region(std::vector<Rc<Profile>>&& slices, const ViewParams& params)
{
    _profiles = std::move(slices);
    _params = params;
    _maxDistance = 0;

    updateData();
}

void Region::updateData()
{
    double viewArea = 0;
    double hitArea = 0;

    RegionBuilder viewBuilder(_params.angleStep, !_params.isBidirectional);
    RegionBuilder hitBuilder(_params.angleStep, !_params.isBidirectional);

    for (const Rc<Profile>& prof : _profiles) {
        _maxDistance = std::max(std::max(_maxDistance, prof->maxViewDistance()), prof->maxHitDistance());

        for (const Interval& i : prof->horizontalVisionIntervals()) {
            viewArea += calcArea(i, _params.angleStep);
        }
        viewBuilder.allIntervals.emplace_back(prof->direction(), prof->horizontalVisionIntervals());

        for (const Hit& i : prof->hits()) {
            hitArea += calcArea(i.interval(), _params.angleStep);
        }
        hitBuilder.allIntervals.emplace_back(prof->direction(), prof->horizontalHitIntervals());
    }
    _maxDistance = std::max(_maxDistance, _params.maxHitDistance);
    _maxDistance = std::max(_maxDistance, _params.maxBeamDistance);
    //_maxDistance *= 1.0 + _params.additionalDistancePercent / 100.0;

    _curves = viewBuilder.calcAll();
    _hitCurves = hitBuilder.calcAll();

    double angle = 360;
    if (!_params.isBidirectional) {
        angle = _params.maxAzimuth - _params.minAzimuth;
        if (angle < 0) {
            angle = 360 + angle;
        }
    }

    _viewCoeff = viewArea / (circleArea(_params.maxBeamDistance, angle) - circleArea(_params.minBeamDistance, angle));
    _hitCoeff = hitArea / (circleArea(_params.maxHitDistance, angle) - circleArea(_params.minHitDistance, angle));

    //BMCL_DEBUG() << _curves.size() << " " << _hitCurves.size();
}

Region::~Region()
{
}

const std::vector<Rc<Profile>>& Region::profiles() const
{
    return _profiles;
}

const std::vector<PointVector>& Region::curves() const
{
    return _curves;
}

const std::vector<PointVector>& Region::hitCurves() const
{
    return _hitCurves;
}

const ViewParams& Region::params() const
{
    return _params;
}

double Region::maxDistance() const
{
    return _maxDistance;
}

double Region::viewCoeff() const
{
    return _viewCoeff;
}

double Region::hitCoeff() const
{
    return _hitCoeff;
}
}
