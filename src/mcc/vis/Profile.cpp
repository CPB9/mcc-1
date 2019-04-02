#include "mcc/vis/Profile.h"
#include "mcc/vis/RadarParams.h"
#include "mcc/vis/Point.h"

#include <bmcl/Option.h>
#include <bmcl/Logging.h>
#include <bmcl/Math.h>

#include <QDebug>

#include <cmath>
#include <cassert>
#include <functional>
#include <algorithm>

namespace mccvis {

constexpr const double earthRadius = 6371110;
constexpr const double frenelKoeff = 0.8 * 17.3; // min zona frenelya (http://www.gigabitradio.ru/zony-frenelya/)

static inline double frenelR(double freqMhz, double d1m, double d2m)
{
    return frenelKoeff * std::sqrt(d1m * d2m / (d1m + d2m) / freqMhz);
}

static bmcl::Option<Point> intersect(const Point& a1, const Point& a2, const Point& b1, const Point& b2)
{
    double bx = a2.x() - a1.x();
    double by = a2.y() - a1.y();
    double dx = b2.x() - b1.x();
    double dy = b2.y() - b1.y();
    double perp = bx * dy - by * dx;
    if (perp == 0) { //FIXME: сравнение даблов
        return bmcl::None;
    }
    double cx = b1.x() - a1.x();
    double cy = b1.y() - a1.y();
    double t = (cx * dy - cy * dx) / perp;
    if (t < 0 || t > 1) {
        return bmcl::None;
    }
    double u = (cx * by - cy * bx) / perp;
    if (u < 0 || u > 1) {
        return bmcl::None;
    }
    return Point(std::fma(t, bx, a1.x()), std::fma(t, by, a1.y()));
}

constexpr const double pi = bmcl::pi<double>();

static inline Point findEdgeIntersection(const ViewParams& params, const Point& start, double k)
{
    double k2 = std::sqrt(k*k + 1);
    double xx = params.maxBeamDistance / k2;
    double x = start.x() + xx;
    double y = start.y() + xx * k;
    return Point(x, y);
}

bool Profile::findProfIntersection(std::vector<Data>::const_iterator it, Point* end)
{
    bmcl::Option<Point> profIntersection = intersect(_rays.start, *end, it->profile, (it + 1)->profile);
    if (profIntersection.isSome()) {
        *end = profIntersection.unwrap();
        return false;
    }
    return true;
}

bool Profile::findProfAndAddTargetIntersection(std::vector<Data>::const_iterator it, Point* end, bool* hasTargetInt)
{
    bmcl::Option<Point> targetIntersection = intersect(_rays.start, *end, it->target, (it + 1)->target);
    if (targetIntersection.isSome()) {
        _intersections.push_back(targetIntersection.unwrap());
    }
    *hasTargetInt = targetIntersection.isSome();
    return findProfIntersection(it, end);
}

void Profile::findRays(const ViewParams& params, double radarY)
{
    double maxk = std::tan(params.maxAngle * pi / 180.0);
    double mink = std::tan(params.minAngle * pi / 180.0);
    _rays.start.setX(_data.at(0).profile.x());
    _rays.start.setY(radarY);
    _rays.minEnd = findEdgeIntersection(params, _rays.start, mink);
    _rays.maxEnd = findEdgeIntersection(params, _rays.start, maxk);
    _rays.minEdge = _rays.minEnd;
    _rays.maxEdge = _rays.maxEnd;

   //TODO: усечение по мин длине луча

    double current = params.minAngle;
    auto it = _data.begin();
    auto iend = _data.end() - 1;
    bool hasTargetInt;
//     while (it->profile.x() > _params.minBeamDistance) {
//         if (it == kend) {
//             return;
//         }
//         it++;
//     }

    _viewRegion.push_back(_rays.start);

    if (_params.canViewGround) {
        for (; it < iend; it++) {
            if (!findProfAndAddTargetIntersection(it, &_rays.minEnd, &hasTargetInt)) {
                _rays.minIsCut = true;
                _viewRegion.push_back(_rays.minEnd);
                if (!findProfAndAddTargetIntersection(it, &_rays.maxEnd, &hasTargetInt)) {
                    _rays.maxIsCut = true;
                    goto alignIntersections;
                }
                it++;
                break;
            }
            if (!findProfAndAddTargetIntersection(it, &_rays.maxEnd, &hasTargetInt)) {
                _rays.maxIsCut = true;
                it++;
                break;
            }
        }
        if (it == _data.begin()) {
            it++;
        }
        for (; it < iend; it++) {
            if (it->profile.x() > _params.maxBeamDistance) {
                break;
            }
            _viewRegion.push_back(it->profile);
            double kp = (it - 1)->kview;
            double k = it->kview;
            double kn = (it + 1)->kview;
            if ((k > kp) && (k > kn) && (k < maxk) && (k > mink) && (k > current)) {
                current = k;
                double correctedK;
                Point peak = it->profile;
                if (_params.useFresnelRegion) {
                    double x = it->profile.x();
                    double dy = frenelR(_params.frequency, x, _params.maxBeamDistance - x);
                    peak.ry() += dy;
                    correctedK = (it->profile.y() - radarY + dy) / x;
                    _viewRegion.push_back(peak);
                } else {
                    correctedK = k;
                }
                Point end = findEdgeIntersection(params, _rays.start, correctedK);
                bool hasInt = false;
                for (; it < iend; it++) {
                    if(!findProfAndAddTargetIntersection(it, &_rays.maxEnd, &hasTargetInt)) {
                        _rays.maxIsCut = true;
                        goto alignIntersections;
                    }
                    if(!findProfAndAddTargetIntersection(it, &end, &hasTargetInt)) {
                        // it++ ?
                        break;
                    }
                    hasInt |= hasTargetInt;
                }
                _viewRegion.push_back(end);
                _rays.ends.emplace_back();
                RayEnd& rayEnd = _rays.ends.back();
                rayEnd.peak = peak;
                rayEnd.end = end;
                rayEnd.hasTargetIntersections = hasInt;
            } else {
                if(!findProfAndAddTargetIntersection(it, &_rays.maxEnd, &hasTargetInt)) {
                    _rays.maxIsCut = true;
                    goto alignIntersections;
                }
            }
        }
    } else {
        it++;
        auto peakIt = it;
        double current = _params.minAngle;
        for (; it < iend; it++) {
            double k = it->kview;
            if (it->profile.x() > _params.maxBeamDistance) {
                break;
            }
            if ((k < maxk) && (k > mink) && (k > current)) {
                current = k;
                peakIt = it;
            }
        }
        if (_params.useFresnelRegion) {
            double x = peakIt->profile.x();
            double dy;
            if (x >= _params.maxBeamDistance) {
                dy = 0;
            } else {
                dy = frenelR(_params.frequency, x, _params.maxBeamDistance - x);
            }
            current = (peakIt->profile.y() - radarY + dy) / x;
        }
        Point end = findEdgeIntersection(params, _rays.start, current);
        const Point& peak = (peakIt + 1)->profile;
        if (current <= _params.minAngle) {
            for (it = _data.begin(); it < iend; it++) {
                if (!findProfAndAddTargetIntersection(it, &_rays.minEnd, &hasTargetInt)) {
                    _rays.minIsCut = true;
                }
                findProfIntersection(it, &end);
                if (!findProfAndAddTargetIntersection(it, &_rays.maxEnd, &hasTargetInt)) {
                    _rays.maxIsCut = true;
                }
            }
        } else if (current < _params.maxAngle) {
            for (it = _data.begin(); it < iend; it++) {
                findProfIntersection(it, &_rays.minEnd);
                findProfAndAddTargetIntersection(it, &end, &hasTargetInt);
                if (!findProfAndAddTargetIntersection(it, &_rays.maxEnd, &hasTargetInt)) {
                    _rays.maxIsCut = true;
                }
            }
        }
        _viewRegion.push_back(end);
        _rays.ends.emplace_back();
        RayEnd& rayEnd = _rays.ends.back();
        rayEnd.peak = peak;
        rayEnd.end = end;
        rayEnd.hasTargetIntersections = true;
    }

alignIntersections:
    if (_params.canViewGround) {
        if (_rays.ends.empty()) {
            Point prof = _data.back().profile;
            _rays.ends.emplace_back(prof, prof, false);
        } else {
            Point end = _rays.ends.back().peak;
            Point start = _rays.start;
            Point prof = _data.back().profile;
            double klast = (end.y() - start.y()) / (end.x() - start.x());
            double k = (prof.y() - start.y()) / (prof.x() - start.x());
            if (k > klast) {
                _rays.ends.emplace_back(prof, prof, false);
            }
        }
    }

    _viewRegion.push_back(_rays.maxEnd);
    _viewRegion.push_back(_rays.start);
    double mind = _params.minBeamDistance;
    double maxd = _params.maxBeamDistance;


    if (_intersections.size() % 2) {
        //TODO: усечение по макс длине луча
        _intersections.push_back(_data.back().target);
    }
    //HACK
    std::sort(_intersections.begin(), _intersections.end(), [](const Point& p1, const Point& p2) {
        return p1.x() < p2.x();
    });

    //REFACT
    std::size_t intsToRemoveFront = 0;
    std::size_t i = 0;
    for (; i < _intersections.size(); i += 2) {
        if (_intersections[i].x() < mind) {
            if (_intersections[i + 1].x() < mind) {
                intsToRemoveFront += 2;
            } else {
                _intersections[i].rx() = mind;
                for (std::size_t j = 0; j < (_data.size() - 1); j++) {
                    const Point& p1 = _data[j].target;
                    const Point& p2 = _data[j + 1].target;
                    if (p1.x() <= mind && p2.x() >= mind) {
                        double k = (p2.y() - p1.y()) / (p2.x() - p1.x());
                        double b = p2.y() - k * p2.x();
                        _intersections[i].ry() = k * mind + b;
                    }
                }
                break;
            }
        } else {
            break;
        }
    }

    std::size_t intsToRemoveBack = 0;
    for (; i < _intersections.size(); i += 2) {
        if (_intersections[i + 1].x() > maxd) {
            if (_intersections[i].x() > maxd) {
                intsToRemoveBack += 2;
            } else {
                _intersections[i + 1].rx() = maxd;
                for (std::size_t j = (_data.size() - 1); j >= 2; j--) {
                    const Point& p1 = _data[j].target;
                    const Point& p2 = _data[j + 1].target;
                    if (p1.x() <= maxd && p2.x() >= maxd) {
                        double k = (p2.y() - p1.y()) / (p2.x() - p1.x());
                        double b = p2.y() - k * p2.x();
                        _intersections[i + 1].ry() = k * maxd + b;
                    }
                }
                break;
            }
        }
    }

    _intersections.erase(_intersections.begin(), _intersections.begin() + intsToRemoveFront);
    _intersections.resize(_intersections.size() - intsToRemoveBack);


    _visionIntervals.reserve(_intersections.size() / 2);
    for (std::size_t i = 0; i < _intersections.size(); i += 2) {
        _visionIntervals.emplace_back(_intersections[i].x(), _intersections[i+1].x());
    }

}

template <typename T, bool isTowards>
struct ItSelector;

template <typename T>
struct ItSelector<T, true> {
    using it = typename std::vector<T>::const_iterator;

    static it begin(const std::vector<T>& vec)
    {
        return vec.cbegin();
    }

    static it end(const std::vector<T>& vec)
    {
        return vec.cend();
    }
};

template <typename T>
struct ItSelector<T, false> {
    using it = typename std::vector<T>::const_reverse_iterator;

    static it begin(const std::vector<T>& vec)
    {
        return vec.crbegin();
    }

    static it end(const std::vector<T>& vec)
    {
        return vec.crend();
    }
};

template <bool isTowards>
struct Comparator {
    template <typename T, typename R>
    static bool more(const T& t, const R& r)
    {
        if (isTowards) {
            return t > r;
        }
        return t < r;
    }

    template <typename T, typename R>
    static bool moreEq(const T& t, const R& r)
    {
        if (isTowards) {
            return t >= r;
        }
        return t <= r;
    }

    template <typename T, typename R>
    static bool less(const T& t, const R& r)
    {
        if (isTowards) {
            return t < r;
        }
        return t > r;
    }

    template <typename T, typename R>
    static bool lessEq(const T& t, const R& r)
    {
        if (isTowards) {
            return t <= r;
        }
        return t >= r;
    }
};

class OnExit {
public:
    explicit OnExit(std::function<void()>&& onExit)
        : _onExit(std::move(onExit))
    {
    }

    ~OnExit()
    {
        _onExit();
    }

private:
    std::function<void()> _onExit;
};

template <bool isTowards>
void Profile::findHits()
{
    if (_data.size() < 2) {
        return;
    }
    if (_visionIntervals.empty()) {
        return;
    }

    Comparator<isTowards> comp;

    auto begin = ItSelector<Data, isTowards>::begin(_data);
    auto end = ItSelector<Data, isTowards>::end(_data);
    auto it = end - 1;

    auto intervalBegin = ItSelector<Interval, isTowards>::begin(_visionIntervals);
    auto intervalEnd = ItSelector<Interval, isTowards>::end(_visionIntervals);
    auto intervalIt = intervalEnd - 1;

    auto rocketBegin = _data.begin();
    //auto rocketEnd = _data.end();
    auto rocketIt = rocketBegin;
    Point rocket;

    double minHit;
    double maxHit;

    if (isTowards) {
        minHit = _params.minHitDistance;
        maxHit = _params.maxHitDistance;
    } else {
        minHit = _params.maxHitDistance;
        maxHit = _params.minHitDistance;
    }

    OnExit onExit([=]() {
        std::sort(_hits.begin(), _hits.end(), [=](const Hit& left, const Hit& right) {
            return left.hit.x() < right.hit.x();
        });
    });

    bool hasEdgeHit = false;
    QPointF edgeHit;
    QPointF edgeDetection;
    auto edgeDetectionIt = it;
    if (true) {
        while (comp.more(it->profile.x(), maxHit)) {
            it--;
            if (it == begin) {
                goto findAllIntervals;
            }
        }
        const QPointF& p1 = it->target;
        const QPointF& p2 = (it + 1)->target;
        double k = (p2.y() - p1.y()) / (p2.x() - p1.x());
        double b = p2.y() - k * p2.x();
        edgeHit.rx() = maxHit;
        edgeHit.ry() = k * maxHit + b;

        rocket.rx() = _data.begin()->target.x();
        rocket.ry() = _radarY;

        while (true) {
            const Point& current = it->target;
            it++;
            if (it >= end) {
                goto findAllIntervals;
            }
            const Point& next = it->target;
            calcNextRocketInterval(current, next, &rocket);
            if (rocket.x() >= maxHit) {
                hasEdgeHit = true;
                edgeDetectionIt = it;
                edgeDetection = it->target;
                break;
            }
        }

    }

findAllIntervals:
    it = end - 1;

    while (true) {
        double currentIntervalStart;
        double currentIntervalEnd;
        if (isTowards) {
            currentIntervalStart = intervalIt->start();
            currentIntervalEnd = intervalIt->end();
        } else {
            currentIntervalStart = intervalIt->end();
            currentIntervalEnd = intervalIt->start();
        }
        double accumTime = 0;
        Point detection;

        while (comp.more(it->profile.x(), currentIntervalEnd)) {
            it--;
            if (it == begin) {
                return;
            }
        }
        if (comp.less(it->profile.x(), minHit)) {
            return;
        }
        while (true) {
            double currentX = it->profile.x();
            it--;
            double nextX = it->profile.x();
            if (comp.less(nextX, currentIntervalStart)) {
                goto selectNextInterval;
            }
            if (comp.less(nextX, minHit)) {
                return;
            }
            double deltaX;
            if (isTowards) {
                deltaX = currentX - nextX;
            } else {
                deltaX = nextX - currentX;
            }
            accumTime += deltaX / _params.targetSpeed;
            if (accumTime >= (_params.reactionTime + _params.externReactionTime)) {
                detection = it->target;
                break;
            }
            if (it == begin) {
                return;
            }
        }

        rocketIt = rocketBegin;
        rocket.rx() = _data.begin()->target.x();
        rocket.ry() = _radarY;
        //PointVector rocketPath;
        //rocketPath.emplace_back(rocketIt->profile.x(), _params.radarHeight);

        while (true) {
            const Point& current = it->target;
            it--;
            const Point& next = it->target;
            if (comp.less(next.x(), currentIntervalStart)) {
                goto selectNextInterval;
            }
            if (comp.less(next.x(), minHit)) {
                return;
            }

            if (rocket.x() < next.x()) { //HACK
                calcNextRocketInterval(current, next, &rocket);
            }
            if (rocket.x() >= next.x()) {
                auto* hits = &_hits;
                bool isOutOfRange = comp.moreEq(next.x(), maxHit);
                if (isOutOfRange) {
                    hits = &_outOfRangeHits;
                    if (isTowards) {
                        hits->emplace(hits->begin(), detection, next, std::max(currentIntervalStart, _params.minHitDistance));
                    } else {
                        hits->emplace_back(detection, next, std::min(currentIntervalStart, _params.maxHitDistance));
                    }
                }
                hits = &_hits;
                QPointF hit = next;
                if (hasEdgeHit
                    && comp.moreEq(edgeHit.x(), currentIntervalStart)
                    && comp.lessEq(edgeHit.x(), currentIntervalEnd)
                    && comp.moreEq(edgeDetection.x(), currentIntervalStart)
                    && comp.lessEq(edgeDetection.x(), currentIntervalEnd)
                ) {
                    double timeLeft = _params.reactionTime + _params.externReactionTime;

                    while (true) {
                        const Point& current = edgeDetectionIt->target;
                        edgeDetectionIt++;
                        if (comp.moreEq(edgeDetectionIt->target.x(), currentIntervalEnd)) {
                            hasEdgeHit = false;
                            break;
                        }
                        const Point& next = edgeDetectionIt->target;
                        double dx = current.x() - next.x();
                        double dy = current.y() - next.y();
                        double targetDistance = std::hypot(dx, dy);
                        timeLeft -= targetDistance / _params.targetSpeed;
                        if (timeLeft <= 0) {
                            break;
                        }
                    }

                    if (hasEdgeHit) {
                        hasEdgeHit = false;
                        detection = edgeDetection;
                        hit = edgeHit;
                        if (isTowards) {
                            hits->emplace(hits->begin(), detection, hit, std::max(currentIntervalStart, _params.minHitDistance));
                        } else {
                            hits->emplace_back(detection, hit, std::min(currentIntervalStart, _params.maxHitDistance));
                        }
                        break;
                    }
                }

                if (!isOutOfRange) {
                    if (isTowards) {
                        hits->emplace(hits->begin(), detection, hit, std::max(currentIntervalStart, _params.minHitDistance));
                    } else {
                        hits->emplace_back(detection, hit, std::min(currentIntervalStart, _params.maxHitDistance));
                    }
                }

                break;
            }
        }

selectNextInterval:
        if (intervalIt == intervalBegin) {
            return;
        }
        intervalIt--;
    }
}

void Profile::calcNextRocketInterval(const Point& current, const Point& next, Point* rocket)
{
    double dx = current.x() - next.x();
    double dy = current.y() - next.y();
    double targetDistance = std::hypot(dx, dy);
    double time = targetDistance / _params.targetSpeed;
    double rocketDistance = time * _params.missleSpeed;
    double rdx = current.x() - rocket->x();
    double rdy = current.y() - rocket->y();
    double rk = rdy / rdx;

    double rocketXDistance = std::sqrt(rocketDistance * rocketDistance / (1 + rk * rk));
    double rocketYDistance = rocketXDistance * rk;
    rocket->rx() += rocketXDistance;
    rocket->ry() += rocketYDistance;
}

template <bool isRelativeHeight, bool hasRefraction>
void Profile::fillData(const PointVector& slice)
{
    for(std::size_t i = 0; i < slice.size(); i++) {
        double l = slice[i].x();
        double h = slice[i].y();
        double dy;

        if (hasRefraction) {
            dy = -l * l / 1000 / 1000 / 16.97;
            h += dy;
        } else {
            double a = l / earthRadius;
            l = std::sin(a) * (earthRadius + h);
            double h2 = -earthRadius + std::cos(a) * (earthRadius + h);
            dy = h2 - h;
            h = h2;
        }

        Data& row = _data[i];
        row.profile.setX(l);
        row.profile.setY(h);
        row.dy = dy;
        if (isRelativeHeight) {
            row.target.setX(row.profile.x());
            row.target.setY(row.profile.y() + _params.objectHeight);
        } else {
            row.target.setX(row.profile.x());
            row.target.setY(std::max(_params.objectHeight + dy, row.profile.y() + 1));
        }
        if (_hmin > row.profile.y()) {
            _hmin = row.profile.y();
        }
        if (_hmax < row.target.y()) {
            _hmax = row.target.y();
        }
    }
}

Profile::Profile(double direction, const PointVector& slice, const ViewParams& params)
{
    _direction = direction;
    if (slice.size() == 0) {
        return;
    }
    _params = params;
    _start = slice[0];
    assert(params.minAngle <= params.maxAngle);
    _data.resize(slice.size());
    double radarY;
    if (params.isRelativeHeight) {
        radarY = _start.y() + params.radarHeight;
    } else {
        radarY = std::max(_start.y() + 0.1, params.radarHeight);
    }
    _radarY = radarY;
    _hmin = radarY;
    _hmax = _hmin;
    // не оптимально
    if (params.isTargetRelativeHeight) {
        if (params.hasRefraction) {
            fillData<true, true>(slice);
        } else {
            fillData<true, false>(slice);
        }
    } else {
        if (params.hasRefraction) {
            fillData<false, true>(slice);
        } else {
            fillData<false, false>(slice);
        }
    }

    //unused Data& first = _data[0];
    double deltaAngleRadians = params.deltaAngle * pi / 180.0;
    static_assert(std::numeric_limits<double>::is_iec559, "iec559 double required");
    _data.front().kview = -std::numeric_limits<double>::infinity();
    for(std::size_t i = 1; i < slice.size(); i++) {
        Data& d = _data[i];
        double k = (d.profile.y() - radarY) / d.profile.x();
        d.kview = std::tan(std::atan(k) + deltaAngleRadians);
    }

    _rect.setLeft(_data.front().profile.x());
    _rect.setTop(_hmax);
    _rect.setRight(_data.back().profile.x());
    _rect.setBottom(_hmin);

    double dy = (_rect.top() - _rect.bottom()) * 0.02;
    _rayRect.setLeft(_rect.left());
    _rayRect.setRight(_rect.right());
    _rayRect.setBottom(_rect.bottom() - dy);
    _rayRect.setTop(_rect.top() + dy);

    findRays(params, radarY);

    if (params.calcHits) {
        if (params.isTargetDirectedTowards) {
            findHits<true>();
            for (const Hit& hit : _hits) {
                _hitIntervals.emplace_back(hit.endX, hit.hit.x());
            }
        } else {
            findHits<false>();
            for (const Hit& hit : _hits) {
                _hitIntervals.emplace_back(hit.hit.x(), hit.endX);
            }
        }
    }

    if (!_rays.ends.empty()) {
        Point delta = _rays.ends.back().peak - _rays.start;
        _viewAngle = bmcl::radian<double>() * std::atan2(delta.y(), delta.x());
        _peakDistance = _rays.ends.back().peak.x();
    } else {
        _viewAngle = 0;
        _peakDistance = 0;
    }
    if (!_visionIntervals.empty()) {
        _minViewDistance = _visionIntervals.front().start();
        _maxViewDistance = _visionIntervals.back().end();
    } else {
        _minViewDistance = 0;
        _maxViewDistance = 0;
    }
    if (!_hitIntervals.empty()) {
        _minHitDistance = _hitIntervals.front().start();
        _maxHitDistance = _hitIntervals.back().end();
    } else {
        _minHitDistance = 0;
        _maxHitDistance = 0;
    }
}

const std::vector<Profile::Data>& Profile::data() const
{
    return _data;
}

const Rays& Profile::rays() const
{
    return _rays;
}

bmcl::Option<std::pair<double, double>> Profile::verticalVisionIntervalAt(double x) const
{
    if (x < _params.minBeamDistance) {
        return bmcl::None;
    }

    auto yAt = [](const Point& start, const Point& end, double x) {
        double k = (end.y() - start.y()) / (end.x() - start.x());
        double b = start.y() - k * start.x();
        return k * x + b;
    };

    auto yAtProf = [this](double x) {
        std::size_t i = 1;
        assert(_data.size() > 1);
        while (i < _data.size() && _data[i].profile.x() < x) {
            i++;
        }
        //return yAt(_data[i - 1].profile, _data[i].profile, x);
        //BMCL_DEBUG() << _data.size() << " " << i;
        //assert(i < _data.size());
        if (i >= _data.size()) {
            i = _data.size() - 1;
        }
        return _data[i - 1].profile.y();
    };

    auto dAtCircle = [](const Point& center, double r, double x) {
        double xx = x - center.x();
        return std::sqrt(r * r - xx * xx);
    };

//     if (_rays.maxIsCut) {
//         if (x > (_rays.maxEnd.x - _rays.start.x)) {
//             return bmcl::None;
//         }
//
//         double top = yAt(_rays.start, _rays.maxEnd, x);
//         double bottom;
//         if (x < (_rays.minEnd.x - _rays.start.x)) {
//             bottom = yAt(_rays.start, _rays.minEnd, x);
//         } else {
//             bottom = yAtProf(x);
//         }
//         return std::make_pair(bottom, top);
//     }

    if (x > _params.maxBeamDistance) {
        return bmcl::None;
    }


    double bottom;
    double top;
    if (!_rays.minIsCut) {
        bool longerThenTop = x > (_rays.maxEnd.x() - _rays.start.x());
        bool longerThenBottom = x > (_rays.minEnd.x() - _rays.start.x());
        if (longerThenTop) {
            double d = dAtCircle(_rays.start, _params.maxBeamDistance, x);
            if (longerThenBottom) {
                bottom = _rays.start.y() - d;
                top = _rays.start.y() + d;
            } else {
                bottom = yAt(_rays.start, _rays.minEnd, x);
                top = _rays.start.y() + d;
            }
            goto end;
        }
        bottom = yAt(_rays.start, _rays.minEnd, x);
        top = yAt(_rays.start, _rays.maxEnd, x);
        goto end;
    }

    if (x > (_rays.maxEnd.x() - _rays.start.x())) {
        double d = dAtCircle(_rays.start, _params.maxBeamDistance, x);
        top = _rays.start.y() + d;
    } else {
        top = yAt(_rays.start, _rays.maxEnd, x);
    }
    if (_rays.ends.empty()) {
        if (x < (_rays.minEnd.x() - _rays.start.x())) {
            bottom = yAt(_rays.start, _rays.minEnd, x);
        } else {
            bottom = yAtProf(x);
        }
    } else {
        for (const RayEnd& end : _rays.ends) {
            if (end.end.x() > x) {
                if (end.peak.x() > x) {
                    bottom = yAtProf(x);
                } else {
                    bottom = yAt(_rays.start, end.end, x);
                }
                goto end;
            }
        }
        bottom = yAtProf(x);
    }

end:
    auto reverseEarthCorrection = [](double x, double y) {
        return std::sqrt(y * y + 2 * earthRadius * y + x * x + earthRadius * earthRadius) - earthRadius;
        //return y;
    };
    return std::make_pair(reverseEarthCorrection(x, bottom), reverseEarthCorrection(x, top));
}

const IntervalVector& Profile::horizontalVisionIntervals() const
{
    return _visionIntervals;
}

const IntervalVector& Profile::horizontalHitIntervals() const
{
    return _hitIntervals;
}


const PointVector& Profile::viewIntersections() const
{
    return _intersections;
}

const PointVector& Profile::viewRegion() const
{
    return _viewRegion;
}

Profile::~Profile()
{
}

const std::vector<Hit>& Profile::hits() const
{
    return _hits;
}

const std::vector<Hit>& Profile::outOfRangeHits() const
{
    return _outOfRangeHits;
}
}
