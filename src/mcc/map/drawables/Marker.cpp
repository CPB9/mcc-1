#include "mcc/map/drawables/Marker.h"

#include <QPixmap>
#include <QPainter>

namespace mccmap {

Marker::Marker(const QPointF& position, Qt::Alignment alignment)
    : MarkerBase(alignment)
    , _point(position)
{
}

Marker::~Marker()
{
}

void Marker::draw(QPainter* p, const MapRect* rect) const
{
    drawMarker(p, position() - rect->mapOffsetRaw());
}

QRectF Marker::rect() const
{
    return WithRect<Marker>::positionedRect(pixmap().rect(), _point.position() + offset());
}
}
