#include "mcc/map/MapRect.h"
#include "mcc/map/drawables/WithPosition.h"
#include "mcc/map/WebMapProperties.h"
#include "mcc/uav/Structs.h"
#include "mcc/geo/Bbox.h"

#include <bmcl/Math.h>

#include <QWidget>

#include <cmath>

namespace mccmap {

MapRect::MapRect(const mccgeo::MercatorProjection& proj, QWidget* parent)
    : QObjectRefCountable<QObject>(parent)
    , _proj(proj)
    , _offset(0, 0)
    , _size(0, 0)
    , _maxMapSize(256)
    , _zoomLevel(0)
    , _isCenteredVertically(false)
    , _parent(parent)
    , _modifiers(Qt::NoModifier)
    , _deadZone()
{
}

MapRect::MapRect(QWidget* parent)
    : MapRect(mccgeo::MercatorProjection(mccgeo::MercatorProjection::SphericalMercator), parent)
{
}

QPoint MapRect::toWindowSystemCoordinates(const QPoint& mapOffset) const
{
    return _parent->mapToGlobal(mapOffset - this->mapOffsetFull());
}

QPoint MapRect::fromWindowSystemCoordinates(const QPoint& pos) const
{
    return _parent->mapFromGlobal(pos) + mapOffsetFull();
}

void MapRect::execWidget(const QPointF& mapPos, QWidget* widget)
{
    Q_UNUSED(mapPos);
    QPoint pos = QCursor::pos();
    widget->setParent(parent());
    widget->setWindowFlags(Qt::Window);
    widget->move(pos);
    widget->setWindowModality(Qt::ApplicationModal);
    widget->show();
}

void MapRect::resize(const QSize& size)
{
    _size = size;
    adjustMaxSize(_zoomLevel);
    adjustMapOffsets();
}

void MapRect::scroll(int dx, int dy)
{
    _offset.rx() -= dx;
    _offset.ry() -= dy;
    adjustMapOffsets();
}

void MapRect::zoom(const QPoint& p, int angle)
{
    int zoomLevel = _zoomLevel + angle;
    if (zoomLevel < 0) {
        return;
    }
    _maxMapSize = std::exp2(zoomLevel) * 256;
    QPoint zoomPoint = _offset + p;
    zoomPoint *= std::exp2(angle);
    _offset = zoomPoint - p;
    _offset.rx() += _maxMapSize * (p.x() / _maxMapSize);
    adjustMaxSize(zoomLevel);
    adjustMapOffsets();
    _zoomLevel = zoomLevel;
    if (_cursorPos.isSome()) {
        _cursorPos.unwrap() *= std::exp2(angle);
    }
}

void MapRect::adjustMapOffsets()
{
    _offset.rx() += _maxMapSize;
    _offset.rx() %= _maxMapSize;
    if (_isCenteredVertically) {
        _offset.setY(-(_size.height() - _maxMapSize) / 2);
    } else {
        if (_offset.y() < 0) {
            _offset.setY(0);
        } else if (_offset.y() + _size.height() > _maxMapSize) {
            _offset.setY(_maxMapSize - _size.height());
        }
    }
}

double MapRect::relativeOffsetY(int y) const
{
    double relativeMapOffset;
    if (_isCenteredVertically) {
        relativeMapOffset = 0; // FIXME
    } else {
        relativeMapOffset = 0.5 - double(_offset.y() + y) / double(_maxMapSize);
    }
    return relativeMapOffset;
}

double MapRect::horisontalPixelLength(int y) const
{
    double yCoord = relativeOffsetY(y) * _proj.mapWidth();
    double maxWidthInPixels = _maxMapSize;
    return _proj.mapWidth() / maxWidthInPixels / _proj.scalingFactorFromY(yCoord);
}

int MapRect::fromRelativeOffsetX(double rx) const
{
    return (1 + rx) * (_maxMapSize) / 2;
}

int MapRect::fromRelativeOffsetY(double ry) const
{
    return (0.5 - ry) * (_maxMapSize);
}

QPointF MapRect::mapOffset(const mccgeo::LatLon& latLon) const
{
    double rx = _proj.longitudeToRelativeOffset(latLon.longitude());
    double ry = _proj.latitudeToRelativeOffset(latLon.latitude());
    QPointF xy;
    xy.rx() = (1 + rx) * (_maxMapSize) / 2;
    xy.ry() = (0.5 - ry) * (_maxMapSize);
    return xy;
}

void MapRect::centerOn(double lat, double lon)
{
    double rx = _proj.longitudeToRelativeOffset(lon);
    double ry = _proj.latitudeToRelativeOffset(lat);
    double x = (1 + rx) * (_maxMapSize) / 2 - _size.width() / 2;
    double y = (0.5 - ry) * (_maxMapSize) - _size.height() / 2;
    _offset.setX(x);
    _offset.rx() += _maxMapSize * (_size.width() / 2 / _maxMapSize);
    _offset.setY(y);
    adjustMapOffsets();
}

void MapRect::centerOn(double lat, double lon, int zoomLevel)
{
    _zoomLevel = zoomLevel;
    adjustMaxSize(zoomLevel);
    centerOn(lat, lon);
}

void MapRect::setProjection(const mccgeo::MercatorProjection& proj)
{
    double relativeMapOffset = relativeOffsetY(_size.height() / 2);
    double lat = _proj.relativeOffsetToLatitude(relativeMapOffset);
    _proj = proj;
    relativeMapOffset = _proj.latitudeToRelativeOffset(lat);
    relativeMapOffset = 0.5 - relativeMapOffset;
    _offset.ry() = relativeMapOffset * _maxMapSize - _size.height() / 2;
}

int MapRect::mapOffsetY(int y) const
{
    int py;
    if (_isCenteredVertically) {
        py = -(_size.height() - _maxMapSize) / 2 + y;
    } else {
        py = _offset.y() + y;
    }
    return py;
}

double MapRect::angleBetween(const mccgeo::LatLon& first, const mccgeo::LatLon& second) const
{
    QPointF a = WithPosition<>::fromLatLon(first, this);
    QPointF b = WithPosition<>::fromLatLon(second, this);
    return WithPosition<>::angleBetween(a, b);
}

mccgeo::LatLon MapRect::latLonOnLine(const mccgeo::LatLon& latLon, double angle, double mapOffset) const
{
    angle = bmcl::degreesToRadians(angle);
    QPointF p = WithPosition<>::fromLatLon(latLon, this);
    QPointF next = p + QPointF(std::cos(angle) * mapOffset, -std::sin(angle) * mapOffset);
    return WithPosition<>::toLatLon(next, this);
}

void MapRect::centerOn(const mccgeo::Bbox& box)
{
    QPointF topLeft = WithPosition<>::fromLatLon(box.topLeft(), this);
    QPointF bottomRight = WithPosition<>::fromLatLon(box.bottomRight(), this);
    double dy = std::abs(topLeft.y() - bottomRight.y());
    double dx = std::abs(topLeft.x() - bottomRight.x());
    double ratio = std::max(dx / _size.width(), dy / _size.height());
    mccgeo::LatLon center = box.center();
    int zoomDelta = std::ceil(std::log2(ratio));
    int newZoom = _zoomLevel - zoomDelta;
    newZoom = std::min<int>(newZoom, WebMapProperties::maxZoom());
    newZoom = std::max<int>(newZoom, WebMapProperties::minZoom());
    centerOn(center.latitude(), center.longitude(), newZoom);
}

mccgeo::LatLon MapRect::centerLatLon() const
{
    mccgeo::LatLon latLon;
    latLon.latitude() = lat(size().height() / 2);
    latLon.longitude() = lon(size().width() / 2);
    return latLon;
}

mccgeo::LatLon MapRect::latLon(const QPoint& pos) const
{
    mccgeo::LatLon latLon;
    latLon.latitude() = lat(pos.y());
    latLon.longitude() = lon(pos.x());
    return latLon;
}

QRectF MapRect::visibleMapRect() const
{
    return QRectF(mapOffset(), _size);
}

MapRect::~MapRect()
{
}

MapRectPluginData::MapRectPluginData(const MapRect* rect)
    : mccplugin::PluginData(id)
    , _rect(rect)
{
}

MapRectPluginData::~MapRectPluginData()
{
}

const MapRect* MapRectPluginData::rect() const
{
    return _rect.get();
}

void MapRect::setParent(QWidget* parent)
{
    _parent = parent;
    //QObject::setParent(parent);
}

void MapRect::setZoomLevel(int zoomLevel)
{
    int zoomDelta = zoomLevel - _zoomLevel;
    zoom(QPoint(_size.width() / 2, _size.height() / 2), zoomDelta);
}
}
