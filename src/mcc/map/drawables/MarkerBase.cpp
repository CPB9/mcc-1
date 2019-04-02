#include "mcc/map/drawables/MarkerBase.h"
#include "mcc/map/drawables/WithPosition.h"

#include <QPainter>

namespace mccmap {

MarkerBase::MarkerBase(Qt::Alignment alignment)
    : _alignment(alignment)
{
    recalcOffset();
}

MarkerBase::~MarkerBase()
{
}

void MarkerBase::drawMarker(QPainter* p, const QPointF& position) const
{
    p->drawPixmap((position + _offset).toPoint(), _pixmap);
}

void MarkerBase::recalcOffset()
{
    _offset = WithPosition<>::edgePoint(_pixmap.size(), _alignment);
}

void MarkerBase::setAlignment(Qt::Alignment alignment)
{
    _alignment = alignment;
    recalcOffset();
}

void MarkerBase::setPixmap(const QPixmap& pixmap)
{
    _pixmap = pixmap;
    recalcOffset();
}
}
