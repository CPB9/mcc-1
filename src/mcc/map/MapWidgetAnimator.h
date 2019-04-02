#pragma once

#include "mcc/Config.h"

#include <QObject>
#include <QPointF>

#include <memory>
#include <chrono>

class QTimer;

namespace mccmap {

class MapWidget;

class MCC_MAP_DECLSPEC MapWidgetAnimator : public QObject {
    Q_OBJECT
public:
    MapWidgetAnimator(MapWidget* parent = 0);
    ~MapWidgetAnimator();

    void start();
    void stop();
    void startManual(const QPointF& speed, double decrement = 2);

private slots:
    void setPosition();

private:
    MapWidget* _parent;
    std::unique_ptr<QTimer> _registerPosTimer;
    std::unique_ptr<QTimer> _animateTimer;
    QPointF _pos;
    QPointF _accumulator;
    QPointF _startSpeed;
    QPointF _speed;
    QPointF _speedDecrement;
    double _minSpeed;
    double _decrement;
    double _decrement0;
    std::chrono::high_resolution_clock::time_point _startTime;
};
}
