#include "mcc/map/drawables/Point.h"
#include "mcc/map/MapRect.h"
#include <QPainter>

namespace mccmap {

void Point::changeZoomLevel(int from, int to)
{
    double ratio = calcZoomRatio(from, to);
    setPosition(_position * ratio);
}

void Point::draw(QPainter* p, const MapRect* rect) const
{
    p->drawPoint(_position - rect->mapOffsetRaw());
}

}
