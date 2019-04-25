#include "mcc/map/FollowAnimator.h"
#include "mcc/map/MapWidget.h"
#include "mcc/map/MapRect.h"

#include <QTimer>

#include <cmath>

namespace mccmap {

template <typename T> int sign(T val) {
    return (T(0) < val) - (val < T(0));
}

static QPointF avg(const FollowAnimator::ContainerType<QPointF>& x) {
    return std::accumulate(x.begin(), x.end(), QPointF(0, 0)) / x.size();
}

FollowAnimator::FollowAnimator(MapWidget* parent)
    : QObject(parent)
    , _parent(parent)
    , _animateTimer(new QTimer(this))
    , _tick(16)
    , _step(0, 0)
    , _historyLen(7)
    , _drsRelativeLen(25)
    , _addStepsLen(10)
{
    _drsRelative = FollowAnimator::ContainerType<QPointF>(_drsRelativeLen, QPointF(0, 0));
    _addSteps = FollowAnimator::ContainerType<QPointF>(_addStepsLen, QPointF(0, 0));
    connect(_animateTimer.get(), &QTimer::timeout, this, [this]() {

        double immobility_radius = 0.00; // relative to disp size

        //main steps
        QPointF avg_object_step(
            1.10 * _avgDx.x() * _tick / _avgDt,
            1.10 * _avgDx.y() * _tick / _avgDt
        );

        //adj steps
        QPointF dr = _timeAndPositions.back().position - _parent->mapRect()->centerMapOffset();
        QSize _disp_size = _parent->mapRect()->size();
        QPointF dr_relative(dr.x() / _disp_size.width(), dr.y() / _disp_size.height());
        _drsRelative.push_back(dr_relative);
        if (_drsRelative.size() >= _drsRelativeLen) {
            _drsRelative.pop_front();
        }

        QPointF avg_drs_relative = avg(_drsRelative);

        QPointF adjustment_to_step(
            0.1 * sign(avg_drs_relative.x()) * (std::abs(avg_drs_relative.x()) - immobility_radius) * _disp_size.width()  * _tick / _avgDt,
            0.1 * sign(avg_drs_relative.y()) * (std::abs(avg_drs_relative.y()) - immobility_radius) * _disp_size.height() * _tick / _avgDt
        );

        //quality moving filter
//         if (dr_relative.x() > 1 || dr_relative.y() > 1) {
//             //
//         }

        //immobility
        if (std::abs(dr_relative.x()) < immobility_radius){
            adjustment_to_step.setX(0);
            avg_object_step.setX(0);
        }
        if (std::abs(dr_relative.y()) < immobility_radius){
            adjustment_to_step.setY(0);
            avg_object_step.setY(0);
        }


        QPointF add_step = avg_object_step + adjustment_to_step;

        _addSteps.push_back(add_step);
        if (_addSteps.size() >= _addStepsLen) {
            _addSteps.pop_front();
        }

        _step += avg(_addSteps);
        _parent->scroll(-_step.toPoint());
        _step -= _step.toPoint();
    });
}

FollowAnimator::~FollowAnimator()
{
}

void FollowAnimator::updateAircraftPosition(const QPointF& pos)
{
    _timeAndPositions.emplace_back(std::chrono::high_resolution_clock::now(), pos);
    if (_timeAndPositions.size() >= _historyLen){
        _timeAndPositions.pop_front();
    }
    if (_timeAndPositions.size() >= 3){
        const TimeAndPosition& back = _timeAndPositions.back();
        const TimeAndPosition& front = _timeAndPositions.front();
        std::size_t size = _timeAndPositions.size() - 1;
        _avgDt = std::chrono::duration_cast<std::chrono::milliseconds>(back.time - front.time).count() / size;
        _avgDx = (back.position - front.position) / size;
        if (!_animateTimer->isActive()){
            _animateTimer->start(_tick);
        }
    }
}

void FollowAnimator::stop()
{
    _animateTimer->stop();
    _timeAndPositions.clear();
    _drsRelative = FollowAnimator::ContainerType<QPointF>(_drsRelativeLen, QPointF(0, 0));
    _addSteps = FollowAnimator::ContainerType<QPointF>(_addStepsLen, QPointF(0, 0));
}

void FollowAnimator::setTimeout(int ms)
{
    _tick = ms;
}
}
