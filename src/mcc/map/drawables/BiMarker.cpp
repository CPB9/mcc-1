#include "mcc/map/drawables/BiMarker.h"

#include <QPainter>

namespace mccmap {

BiMarker::~BiMarker()
{
}

void BiMarker::draw(QPainter* p, const MapRect* rect) const
{
    if (isActive()) {
        _active.drawMarker(p, _point.position() - rect->mapOffsetRaw());
    } else {
        _inactive.drawMarker(p, _point.position() - rect->mapOffsetRaw());
    }
    _overlay.drawMarker(p, _point.position() - rect->mapOffsetRaw());
}

QRectF BiMarker::rect() const
{
    if (isActive()) {
        return WithRect::positionedRect(_active.rect(), _point.position() + _active.offset());
    }
    return WithRect::positionedRect(_inactive.rect(), _point.position() + _inactive.offset());
}
}
