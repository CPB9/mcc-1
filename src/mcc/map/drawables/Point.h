#pragma once

#include "mcc/map/drawables/WithPosition.h"
#include "mcc/map/drawables/Zoomable.h"
#include "mcc/map/drawables/Movable.h"

#include <QPointF>

class QPainter;
class QSizeF;

namespace mccmap {

class MapRect;

class MCC_MAP_DECLSPEC Point : public WithPosition<Point>, public Zoomable<Point>, public Movable<Point> {
public:
    inline Point(const QPointF& position);
    inline Point(double x = 0, double y = 0);

    void changeZoomLevel(int from, int to);
    void draw(QPainter* p, const MapRect* rect) const;

    inline void setPosition(const QPointF& point);
    inline const QPointF& position() const;

private:
    QPointF _position;
};

inline Point::Point(const QPointF& position)
    : _position(position)
{
}

inline Point::Point(double x, double y)
    : _position(x, y)
{
}

inline void Point::setPosition(const QPointF& point)
{
    _position = point;
}

inline const QPointF& Point::position() const
{
    return _position;
}
}
