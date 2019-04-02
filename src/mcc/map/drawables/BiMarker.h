#pragma once

#include "mcc/Config.h"
#include "mcc/map/drawables/Interfaces.h"
#include "mcc/map/drawables/Marker.h"
#include "mcc/map/drawables/Point.h"

#include <QPixmap>
#include <QPainter>
#include <Qt>

class QPainter;

namespace mccmap {

class MCC_MAP_DECLSPEC BiMarker : public AbstractMarker<BiMarker> {
public:
    inline BiMarker();
    inline BiMarker(const QPointF& position);
    ~BiMarker();

    inline void setActive(bool isActive);
    inline bool isActive() const;

    inline void setActiveAlignment(Qt::Alignment alignment);
    inline void setActivePixmap(const QPixmap& pixmap);
    inline const QPixmap& activePixmap() const;
    inline QRectF activeRect() const;

    inline void setInactiveAlignment(Qt::Alignment alignment);
    inline void setInactivePixmap(const QPixmap& pixmap);
    inline const QPixmap& inactivePixmap() const;
    inline QRectF inactiveRect() const;

    inline void setOverlayAlignment(Qt::Alignment alignment);
    inline void setOverlayPixmap(const QPixmap& pixmap);
    inline const QPixmap& overlayPixmap() const;
    inline QRectF overlayRect() const;

    void draw(QPainter* p, const MapRect* rect) const;
    QRectF rect() const;
    inline void changeZoomLevel(int from, int to);

    inline void setPosition(const QPointF& position);
    inline const QPointF& position() const;

protected:
    MarkerBase _active;
    MarkerBase _inactive;
    MarkerBase _overlay;

private:
    Point _point;
    bool _isActive;
};

inline BiMarker::BiMarker(const QPointF& position)
    : _point(position)
    , _isActive(false)
{
}

inline BiMarker::BiMarker()
    : _point(0, 0)
    , _isActive(false)
{
}

inline bool BiMarker::isActive() const
{
    return _isActive;
}

inline void BiMarker::setActive(bool isActive)
{
    _isActive = isActive;
}

inline const QPixmap& BiMarker::activePixmap() const
{
    return _active.pixmap();
}

inline QRectF BiMarker::activeRect() const
{
    return _active.rect();
}

inline const QPixmap& BiMarker::inactivePixmap() const
{
    return _inactive.pixmap();
}

inline QRectF BiMarker::inactiveRect() const
{
    return _inactive.rect();
}

inline void BiMarker::setOverlayAlignment(Qt::Alignment alignment)
{
    _overlay.setAlignment(alignment);
}

inline void BiMarker::setOverlayPixmap(const QPixmap& pixmap)
{
    _overlay.setPixmap(pixmap);
}

inline const QPixmap& BiMarker::overlayPixmap() const
{
    return _overlay.pixmap();
}

inline QRectF BiMarker::overlayRect() const
{
    return _overlay.rect();
}

inline void BiMarker::setActiveAlignment(Qt::Alignment alignment)
{
    _active.setAlignment(alignment);
}

inline void BiMarker::setActivePixmap(const QPixmap& pixmap)
{
    _active.setPixmap(pixmap);
}

inline void BiMarker::setInactiveAlignment(Qt::Alignment alignment)
{
    _inactive.setAlignment(alignment);
}

inline void BiMarker::setInactivePixmap(const QPixmap& pixmap)
{
    _inactive.setPixmap(pixmap);
}

inline const QPointF& BiMarker::position() const
{
    return _point.position();
}

inline void BiMarker::setPosition(const QPointF& position)
{
    _point.setPosition(position);
}

inline void BiMarker::changeZoomLevel(int from, int to)
{
    _point.changeZoomLevel(from, to);
}
}
