#pragma once

#include "mcc/map/MapRect.h"
#include "mcc/ui/CoordinateSystemController.h"
#include "mcc/ui/Fwd.h"
#include "mcc/msg/Nav.h"
#include "mcc/geo/Fwd.h"

#include <bmcl/Math.h>

#include <QString>

#include <type_traits>

namespace mccmap {

MCC_MAP_DECLSPEC void copyToClipboard(QString&& text);

template <typename B = void>
class WithPosition {
public:
    const QPointF& position();
    void setPosition(const QPointF& position);
    void moveBy(const QPointF& delta, const MapRect* rect);
    static void moveBy(const QPointF& delta, const MapRect* rect, QPointF* dest);
    QString printCoordinates(const MapRect* rect, const mccui::CoordinateSystemController* csController);
    void printCoordinatesToClipboard(const MapRect* rect, const mccui::CoordinateSystemController* csController);

    void setLatLon(const mccgeo::LatLon& latLon, const MapRect* rect);
    mccgeo::LatLon toLatLon(const MapRect* rect) const;

    void changeProjection(const MapRect* rect, const mccgeo::MercatorProjection& from, const mccgeo::MercatorProjection& to);

    static QPointF fromLatLon(const mccgeo::LatLon& latLon, const MapRect* rect);
    static QPointF fromLatLon(const mccgeo::LatLon& latLon, const MapRect* rect, const mccgeo::MercatorProjection& proj);
    static mccgeo::LatLon toLatLon(const QPointF& position, const MapRect* rect);
    static mccgeo::LatLon toLatLon(const QPointF& position, const MapRect* rect, const mccgeo::MercatorProjection& proj);
    static double angleBetween(const QPointF& first, const QPointF& second);
    static double angleBetween(const QPointF& p1, const QPointF& c, const QPointF& p2);

    static QPointF edgePoint(const QSizeF& size, Qt::Alignment alignment = Qt::AlignCenter);
};

template <typename B>
inline const QPointF& WithPosition<B>::position()
{
    return static_cast<const B*>(this)->position();
}

template <typename B>
inline void WithPosition<B>::moveBy(const QPointF& delta, const MapRect* rect)
{
    QPointF pos = position();
    moveBy(delta, rect, &pos);
    setPosition(pos);
}

template <typename B>
inline void WithPosition<B>::moveBy(const QPointF& delta, const MapRect* rect, QPointF* pos)
{
    *pos += delta;
    auto mapWidth = rect->maxMapSize();
    if (pos->x() < 0) {
        pos->rx() += mapWidth;
    } else if (pos->x() >= mapWidth) {
        pos->rx() -= mapWidth;
    }
}

template <typename B>
inline void WithPosition<B>::setPosition(const QPointF& position)
{
    static_cast<B*>(this)->setPosition(position);
}

template <typename B>
inline void WithPosition<B>::setLatLon(const mccgeo::LatLon& latLon, const mccmap::MapRect* rect)
{
    static_cast<B*>(this)->setPosition(fromLatLon(latLon, rect));
}

template <typename B>
inline mccgeo::LatLon WithPosition<B>::toLatLon(const mccmap::MapRect* rect) const
{
    return toLatLon(static_cast<const B*>(this)->position(), rect);
}

template <typename B>
inline QPointF WithPosition<B>::fromLatLon(const mccgeo::LatLon& latLon, const mccmap::MapRect* rect)
{
    return fromLatLon(latLon, rect, rect->projection());
}

template <typename B>
inline mccgeo::LatLon WithPosition<B>::toLatLon(const QPointF& position, const mccmap::MapRect* rect)
{
    return toLatLon(position, rect, rect->projection());
}

template <typename B>
QString WithPosition<B>::printCoordinates(const MapRect* rect, const mccui::CoordinateSystemController* csController)
{
    return csController->convertAndFormat(toLatLon(rect), "%1, %2");
}

template <typename B>
inline void WithPosition<B>::printCoordinatesToClipboard(const MapRect* rect, const mccui::CoordinateSystemController* csController)
{
    copyToClipboard(printCoordinates(rect, csController));
}

template <typename B>
inline void WithPosition<B>::changeProjection(const MapRect* rect, const mccgeo::MercatorProjection& from,
                                              const mccgeo::MercatorProjection& to)
{
    mccgeo::LatLon latLon = toLatLon(static_cast<B*>(this)->position(), rect, from);
    static_cast<B*>(this)->setPosition(fromLatLon(latLon, rect, to));
}

template <typename B>
QPointF WithPosition<B>::fromLatLon(const mccgeo::LatLon& latLon, const MapRect* rect,
                                    const mccgeo::MercatorProjection& proj)
{
    double rx = proj.longitudeToRelativeOffset(latLon.longitude());
    double ry = proj.latitudeToRelativeOffset(latLon.latitude());
    int maxMapSize = rect->maxMapSize();
    double x = (1 + rx) * maxMapSize / 2;
    double y = (0.5 - ry) * maxMapSize;
    return QPointF(x, y);
}

template <typename B>
mccgeo::LatLon WithPosition<B>::toLatLon(const QPointF& position, const MapRect* rect,
                                       const mccgeo::MercatorProjection& proj)
{
    int maxMapSize = rect->maxMapSize();
    double rx = position.x() / maxMapSize * 2.0 - 1;
    double ry = 0.5 - position.y() / maxMapSize;
    double lat = proj.relativeOffsetToLatitude(ry);
    double lon = proj.relativeOffsetToLongitude(rx);
    return mccgeo::LatLon(lat, lon);
}

// ось y направлена вниз
template <typename B>
double WithPosition<B>::angleBetween(const QPointF& first, const QPointF& second)
{
    return bmcl::radiansToDegrees(std::atan2(first.y() - second.y(), second.x() - first.x()));
}

template <typename B>
double WithPosition<B>::angleBetween(const QPointF& p1, const QPointF& c, const QPointF& p2)
{
    QPointF v1 = p1 - c;
    QPointF v2 = p2 - c;
    double dp = v1.x() * v2.x() + v1.y() * v2.y();
    double l1 = std::hypot(v1.x(), v1.y());
    double l2 = std::hypot(v2.x(), v2.y());
    return bmcl::radiansToDegrees(std::acos(dp / (l1 * l2)));
}

template <typename B>
QPointF WithPosition<B>::edgePoint(const QSizeF& size, Qt::Alignment alignment)
{
    QPointF p;
    if (alignment & Qt::AlignLeft) {
        p.setX(0);
    } else if (alignment & Qt::AlignRight) {
        p.setX(size.width());
    } else {
        p.setX(size.width() / 2);
    }
    if (alignment & Qt::AlignTop) {
        p.setY(0);
    } else if (alignment & Qt::AlignBottom) {
        p.setY(size.height());
    } else {
        p.setY(size.height() / 2);
    }
    return -p;
}
}
