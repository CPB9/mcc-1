#pragma once

#include "mcc/Config.h"

#include <QPixmap>
#include <Qt>

class QPainter;
class QPointF;

namespace mccmap {

class MCC_MAP_DECLSPEC MarkerBase {
public:
    MarkerBase(Qt::Alignment alignment = Qt::AlignCenter);
    ~MarkerBase();

    void drawMarker(QPainter* p, const QPointF& position) const;
    void setAlignment(Qt::Alignment alignment);
    void setPixmap(const QPixmap& pixmap);
    inline const QPointF& offset() const;
    inline const QPixmap& pixmap() const;
    inline QRectF rect() const;
    Qt::Alignment alignment() const;

private:
    void recalcOffset();
    Qt::Alignment _alignment;
    QPointF _offset;
    QPixmap _pixmap;
};

inline QRectF MarkerBase::rect() const
{
    return _pixmap.rect();
}

inline Qt::Alignment MarkerBase::alignment() const
{
    return _alignment;
}

inline const QPointF& MarkerBase::offset() const
{
    return _offset;
}

inline const QPixmap& MarkerBase::pixmap() const
{
    return _pixmap;
}

}
