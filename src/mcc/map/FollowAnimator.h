#pragma once

#include "mcc/Config.h"

#include <QObject>
#include <QPointF>

#include <memory>
#include <chrono>
#include <deque>

class QTimer;

namespace mccmap {

class MapWidget;

struct AnimationParams {
    double speed;
    double acceleration;
};

class MCC_MAP_DECLSPEC FollowAnimator : public QObject {
public:
    template<class T>
    using ContainerType = std::deque<T>;

    FollowAnimator(MapWidget* parent = 0);
    ~FollowAnimator();

    void updateAircraftPosition(const QPointF& pos);
    void stop();

private:

    MapWidget* _parent;
    std::unique_ptr<QTimer> _animateTimer;

    int _tick;
    QPointF _step;

    struct TimeAndPosition {
        TimeAndPosition(const std::chrono::high_resolution_clock::time_point& time, const QPointF& position)
            : time(time)
            , position(position)
        {
        }

        std::chrono::high_resolution_clock::time_point time;
        QPointF position;
    };

    ContainerType<TimeAndPosition> _timeAndPositions;
    unsigned _history_len;
    double _avg_dt;
    QPointF _avg_dx;

    ContainerType<QPointF> _drs_relative;
    unsigned _drs_relative_len;
    ContainerType<QPointF> _add_steps;
    unsigned _add_steps_len;
};
}
