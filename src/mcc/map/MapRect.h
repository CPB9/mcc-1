#pragma once

#include "mcc/Config.h"
#include "mcc/map/Rc.h"
#include "mcc/geo/MercatorProjection.h"
#include "mcc/map/Fwd.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/QObjectRefCountable.h"
#include "mcc/plugin/PluginData.h"

#include <bmcl/Option.h>

#include <QObject>
#include <QSize>
#include <QPoint>
#include <QRect>

#include <cmath>

class QRectF;
class QWidget;

namespace mccmap {

class MCC_MAP_DECLSPEC MapRect : public mccui::QObjectRefCountable<QObject> {
    Q_OBJECT
public:
    MapRect(const mccgeo::MercatorProjection& proj, QWidget* parent = nullptr);
    explicit MapRect(QWidget* parent = nullptr);
    ~MapRect();

    static inline Rc<MapRect> create(QWidget* parent);

    QPoint toWindowSystemCoordinates(const QPoint& mapOffset) const;
    QPoint fromWindowSystemCoordinates(const QPoint& pos) const;
    inline int zoomLevel() const;
    inline const QSize& size() const;
    inline const QPoint& offset() const;
    QRectF visibleMapRect() const;
    inline int maxMapSize() const;
    QPointF mapOffset(const mccgeo::LatLon& latLon) const;
    inline QPoint mapOffset(int x = 0, int y = 0) const;
    inline QPoint mapOffset(const QPoint& pos) const;
    inline QPoint centerMapOffset() const;
    inline const QPoint& mapOffsetRaw() const;
    inline QPoint mapOffsetFull(int x = 0, int y = 0) const;
    void execWidget(const QPointF& mapPos, QWidget* widget);
    inline int mapOffsetX(int x = 0) const;
    int mapOffsetY(int y = 0) const;
    inline double lon(int x = 0) const;
    inline double lat(int y = 0) const;
    mccgeo::LatLon latLon(const QPoint& pos) const;
    mccgeo::LatLon centerLatLon() const;
    inline const mccgeo::MercatorProjection& projection() const;
    inline bool isCenteredVertically() const;
    double relativeOffsetY(int y) const;
    inline double relativeOffsetX(int x) const;
    int fromRelativeOffsetY(double ry) const;
    int fromRelativeOffsetX(double rx) const;
    double horisontalPixelLength(int y) const;
    mccgeo::LatLon latLonOnLine(const mccgeo::LatLon& latLon, double angle, double mapOffset) const;

    void centerOn(const mccgeo::Bbox& box);
    void centerOn(double lat, double lon);
    void centerOn(double lat, double lon, int zoomLevel);
    void setProjection(const mccgeo::MercatorProjection& proj);
    void setZoomLevel(int zoomLevel);
    void scroll(int dx, int dy);
    void zoom(const QPoint& p, int angle);
    inline void resize(int width, int height);
    void resize(const QSize& size);
    inline Qt::KeyboardModifiers modifiers() const;
    inline void setModifiers(Qt::KeyboardModifiers modifiers);
    inline void setCursorPosition(const QPoint& pos);
    inline void setCursorPosition(bmcl::NoneType);
    inline void resetCursorPosition();
    inline const bmcl::Option<QPoint>& cursorPosition() const;

    double angleBetween(const mccgeo::LatLon& first, const mccgeo::LatLon& second) const;

    inline QWidget* parent() const;
    void setParent(QWidget* parent);

    void setDeadZone(const QRect& zone) {_deadZone = zone;}
    const QRect& deadZone() const {return _deadZone;}

signals:
    void viewChanged();

private:
    inline void adjustMaxSize(int zoomLevel);
    void adjustMapOffsets();

    mccgeo::MercatorProjection _proj;
    QPoint _offset;
    QSize _size;
    bmcl::Option<QPoint> _cursorPos;
    int _maxMapSize;
    int _zoomLevel;
    bool _isCenteredVertically;
    QWidget* _parent;
    Qt::KeyboardModifiers _modifiers;

    // Rects for control widgets
    QRect _deadZone;
};

inline QWidget* MapRect::parent() const
{
    return _parent;
}

inline Rc<MapRect> MapRect::create(QWidget* parent)
{
    return new MapRect(parent);
}

inline void MapRect::resize(int width, int height)
{
    resize(QSize(width, height));
}

inline int MapRect::zoomLevel() const
{
    return _zoomLevel;
}

inline void MapRect::adjustMaxSize(int zoomLevel)
{
    _maxMapSize = std::exp2(zoomLevel) * 256;
    _isCenteredVertically = _maxMapSize < _size.height();
}

inline double MapRect::relativeOffsetX(int x) const
{
    double offset = _offset.x() + x;
    return offset / double(_maxMapSize) * 2.0 - 1;
}

inline double MapRect::lat(int y) const
{
    double ry = relativeOffsetY(y);
    return _proj.relativeOffsetToLatitude(ry);
}

inline double MapRect::lon(int x) const
{
    double rx = relativeOffsetX(x);
    double lon = _proj.relativeOffsetToLongitude(rx);
    lon = std::fmod(lon + 180, 360);
    if (lon < 0) {
        lon += 360;
    }
    return lon - 180;
}

inline int MapRect::mapOffsetX(int x) const
{
    return (_offset.x() + x) % _maxMapSize;
}

inline QPoint MapRect::mapOffsetFull(int x, int y) const
{
    return QPoint(_offset.x() + x, mapOffsetY(y));
}

inline QPoint MapRect::mapOffset(int x, int y) const
{
    return QPoint(mapOffsetX(x), mapOffsetY(y));
}

inline QPoint MapRect::mapOffset(const QPoint& pos) const
{
    return QPoint(mapOffsetX(pos.x()), mapOffsetY(pos.y()));
}

inline bool MapRect::isCenteredVertically() const
{
    return _isCenteredVertically;
}

inline const QPoint& MapRect::offset() const
{
    return _offset;
}

inline const QSize& MapRect::size() const
{
    return _size;
}

inline int MapRect::maxMapSize() const
{
    return _maxMapSize;
}

inline const mccgeo::MercatorProjection& MapRect::projection() const
{
    return _proj;
}

inline const QPoint& MapRect::mapOffsetRaw() const
{
    return _offset;
}

inline void MapRect::setModifiers(Qt::KeyboardModifiers modifiers)
{
    _modifiers = modifiers;
}

inline Qt::KeyboardModifiers MapRect::modifiers() const
{
    return _modifiers;
}

inline const bmcl::Option<QPoint>& MapRect::cursorPosition() const
{
    return _cursorPos;
}

inline void MapRect::resetCursorPosition()
{
    _cursorPos = bmcl::None;
}

inline void MapRect::setCursorPosition(const QPoint& pos)
{
    _cursorPos = pos;
}

inline void MapRect::setCursorPosition(bmcl::NoneType)
{
    _cursorPos = bmcl::None;
}

inline QPoint MapRect::centerMapOffset() const
{
    return QPoint(_offset.x() + _size.width() / 2, _offset.y() + _size.height() / 2);
}

class MCC_MAP_DECLSPEC MapRectPluginData : public mccplugin::PluginData {
public:
    static constexpr const char* id = "mcc::MapRectPluginData";

    MapRectPluginData(const MapRect* rect);
    ~MapRectPluginData();

    const MapRect* rect() const;

private:
    Rc<const MapRect> _rect;
};
}
