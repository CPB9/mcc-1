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
    , _history_len(7)
    , _drs_relative_len(25)
    , _add_steps_len(10)
{
    _drs_relative = FollowAnimator::ContainerType<QPointF>(_drs_relative_len, QPointF(0, 0));
    _add_steps = FollowAnimator::ContainerType<QPointF>(_add_steps_len, QPointF(0, 0));
    connect(_animateTimer.get(), &QTimer::timeout, this, [this]() {

        double immobility_radius = 0.00; // relative to disp size

        //main steps
        QPointF avg_object_step(
            1.10 * _avg_dx.x() * _tick / _avg_dt,
            1.10 * _avg_dx.y() * _tick / _avg_dt
        );

        //adj steps
        QPointF dr = _timeAndPositions.back().position - _parent->mapRect()->centerMapOffset();
        QSize _disp_size = _parent->mapRect()->size();
        QPointF dr_relative(dr.x() / _disp_size.width(), dr.y() / _disp_size.height());
        _drs_relative.push_back(dr_relative);
        if (_drs_relative.size() >= _drs_relative_len) {
            _drs_relative.pop_front();
        }

        QPointF avg_drs_relative = avg(_drs_relative);

        QPointF adjustment_to_step(
            0.1 * sign(avg_drs_relative.x()) * (std::abs(avg_drs_relative.x()) - immobility_radius) * _disp_size.width()  * _tick / _avg_dt,
            0.1 * sign(avg_drs_relative.y()) * (std::abs(avg_drs_relative.y()) - immobility_radius) * _disp_size.height() * _tick / _avg_dt
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

        _add_steps.push_back(add_step);
        if (_add_steps.size() >= _add_steps_len) {
            _add_steps.pop_front();
        }

        _step += avg(_add_steps);
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
    if (_timeAndPositions.size() >= _history_len){
        _timeAndPositions.pop_front();
    }
    if (_timeAndPositions.size() >= 3){
        const TimeAndPosition& back = _timeAndPositions.back();
        const TimeAndPosition& front = _timeAndPositions.front();
        std::size_t size = _timeAndPositions.size() - 1;
        _avg_dt = std::chrono::duration_cast<std::chrono::milliseconds>(back.time - front.time).count() / size;
        _avg_dx = (back.position - front.position) / size;
        if (!_animateTimer->isActive()){
            _animateTimer->start(_tick);
        }
    }
}

void FollowAnimator::stop()
{
    _animateTimer->stop();
    _timeAndPositions.clear();
    _drs_relative = FollowAnimator::ContainerType<QPointF>(_drs_relative_len, QPointF(0, 0));
    _add_steps = FollowAnimator::ContainerType<QPointF>(_add_steps_len, QPointF(0, 0));
}
}
