#pragma once

#include "mcc/Config.h"
#include "mcc/map/drawables/Interfaces.h"
#include "mcc/map/drawables/Point.h"

#include <QPixmap>

namespace mccmap {

class MCC_MAP_DECLSPEC RulerLabel : public AbstractMarker<RulerLabel> {
public:
    inline RulerLabel(const QPointF& position, double distance, double rotataion);

    void draw(QPainter* p, const MapRect* rect) const;
    inline QRectF rect() const;
    inline void changeZoomLevel(int from, int to);

    inline const QPointF& position() const;
    inline void setPosition(const QPointF& position);

    inline void setVisible(bool isVisible);

    void setDistanceAndRotation(double distance, double rotation);

private:
    double _rotation;
    QPixmap _pixmap;
    Point _point;
    QPointF _offset;
    bool _isVisible;
};

inline const QPointF& RulerLabel::position() const
{
    return _point.position();
}

inline void RulerLabel::setVisible(bool isVisible)
{
    _isVisible = isVisible;
}


inline void RulerLabel::setPosition(const QPointF& position)
{
    _point.setPosition(position);
}

inline void RulerLabel::changeZoomLevel(int from, int to)
{
    _point.changeZoomLevel(from, to);
}

inline RulerLabel::RulerLabel(const QPointF& position, double distance, double rotataion)
    : _point(position)
    , _isVisible(true)
{
    setDistanceAndRotation(distance, rotataion);
}

inline QRectF RulerLabel::rect() const
{
    return WithRect::positionedRect(_pixmap.rect(), _point.position());
}
}
