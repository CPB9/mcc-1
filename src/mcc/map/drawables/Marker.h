#pragma once

#include "mcc/Config.h"
#include "mcc/map/drawables/MarkerBase.h"
#include "mcc/map/drawables/Interfaces.h"
#include "mcc/map/drawables/Point.h"

class QPainter;
class QPixmap;

namespace mccmap {

class MCC_MAP_DECLSPEC Marker : public MarkerBase, public AbstractMarker<Marker> {
public:
    Marker(const QPointF& position = QPointF(0, 0), Qt::Alignment alignment = Qt::AlignCenter);
    ~Marker();

    void draw(QPainter* p, const MapRect* rect) const;
    QRectF rect() const;

    inline void changeZoomLevel(int from, int to);
    inline const QPointF& position() const;
    inline void setPosition(const QPointF& position);

    inline QPointF alignedPosition() const;

private:
    Point _point;
};

inline void Marker::setPosition(const QPointF& position)
{
    _point.setPosition(position);
}

inline const QPointF& Marker::position() const
{
    return _point.position();
}

inline void Marker::changeZoomLevel(int from, int to)
{
    _point.changeZoomLevel(from, to);
}

inline QPointF Marker::alignedPosition() const
{
    return _point.position() + offset();
}
}
