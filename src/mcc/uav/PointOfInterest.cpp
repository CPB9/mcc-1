#include "mcc/uav/PointOfInterest.h"
#include "mcc/res/Resource.h"

#include <QApplication>
#include <QDebug>
#include <QStyle>
#include <QPainter>

namespace mccuav {

PointOfInterest::~PointOfInterest()
{
}

QPixmap rotatePixmap(const QPixmap& p, double degrees)
{
    QTransform t;
    t.translate(p.width() / 2.0, p.height() / 2.0);
    t.rotate(degrees);
    t.translate(-p.width() / 2.0, -p.height() / 2.0);
    return p.transformed(t, Qt::SmoothTransformation);
}

PointOfInterest::PointOfInterest(const QString& info, Type type, const mccgeo::LatLon& latLon)
    : PointOfInterest()
{
    setLatLon(latLon);
    setInfo(info);
    setIcon(type);
}

PointOfInterest::PointOfInterest()
    : _icon(-1)
    , _altitude(0.0)
    , _azimuth(0.0)
    , _elevation(0.0)
    , _useAzimuth(false)
    , _useAngle(false)
    , _editable(true)
    , _visible(true)
    , _updatePending(false)
    , _isEnabled(true)
    , _isMoving(false)
    , _baseColor(Qt::white)
    , _useBaseColor(true)
{
    connect(this, &PointOfInterest::uploadToUav, this, [this]() {_updatePending = true; emit pixmapChanged(); });
    connect(this, &PointOfInterest::latLonChanged, this, &PointOfInterest::updated);
    connect(this, &PointOfInterest::azimuthChanged, this, &PointOfInterest::updated);
    connect(this, &PointOfInterest::elevationChanged, this, &PointOfInterest::updated);
    connect(this, &PointOfInterest::altitudeChanged, this, &PointOfInterest::updated);
    connect(this, &PointOfInterest::visibleChanged, this, &PointOfInterest::updated);
}

void PointOfInterest::setIcon(int icon)
{
    Type iconType = static_cast<Type>(icon);
    _icon = icon;
    _kind = iconType;
    QString iconPath;
    switch (iconType)
    {
    case mccuav::PointOfInterest::Runway:
        iconPath = ":/poi/runway.svg";
        break;
    case mccuav::PointOfInterest::Target:
        iconPath = ":/poi/target.svg";
        break;
    case mccuav::PointOfInterest::Parachute:
        iconPath = ":/poi/parachute.svg";
        break;
    case mccuav::PointOfInterest::ParachuteWithPayload:
        iconPath = ":/poi/parachute_payload.svg";
        break;
    case mccuav::PointOfInterest::Bomb:
        iconPath = ":/poi/bomb.svg";
        break;
    case mccuav::PointOfInterest::Missile:
        iconPath = ":/poi/missile.svg";
        break;
    case mccuav::PointOfInterest::CruiseMissile:
        iconPath = ":/poi/cruise_missile.svg";
        break;
    case mccuav::PointOfInterest::Antenna:
        iconPath = ":/poi/antenna.svg";
        break;
    default:
        break;
    }

    if (!iconPath.isEmpty())
    {
        _inactivePixmap = QPixmap::fromImage(mccres::renderSvg(iconPath, 32, 32));
    }
    else
        _inactivePixmap = QApplication::style()->standardPixmap(QStyle::SP_TrashIcon);

    if(_useBaseColor)
        _inactivePixmap = mccui::GraphicsEffectCreator::applyColorEffect(_inactivePixmap, _baseColor, 0.9f);
    createPixmaps();
    emit pixmapChanged();
}

int PointOfInterest::icon() const
{
    return _icon;
}

const QPixmap PointOfInterest::activePixmap() const
{
    if (!_isEnabled)
    {
        if (useAzimuth())
            return rotatePixmap(_grayPixmap, _azimuth);
        return _grayPixmap;
    }

    if (!_updatePending)
    {
        if (useAzimuth())
            return rotatePixmap(_activePixmap, _azimuth);

        return _activePixmap;
    }

    return _grayPixmap;
}

const QPixmap PointOfInterest::inactivePixmap() const
{
    if (!_isEnabled)
    {
        if (useAzimuth())
            return rotatePixmap(_grayPixmap, _azimuth);
        return _grayPixmap;
    }

    if (!_updatePending)
    {
        if (useAzimuth())
            return rotatePixmap(_inactivePixmap, _azimuth);
        return _inactivePixmap;
    }
    return _grayPixmap;
}

void PointOfInterest::createPixmaps()
{
    _activePixmap = mccui::GraphicsEffectCreator::applyColorEffect(_inactivePixmap, Qt::yellow, 0.8f);
    _grayPixmap = mccui::GraphicsEffectCreator::applyColorEffect(_inactivePixmap, Qt::gray);
}

void PointOfInterest::setBaseColor(const QColor& color)
{
    if(_useBaseColor)
        _baseColor = color;
    setIcon(_icon);
}

const QString& PointOfInterest::info() const
{
    return _info;
}

bool PointOfInterest::useAzimuth() const
{
    return _useAzimuth;
}

bool PointOfInterest::useElevation() const
{
    return _useAngle;
}

bool PointOfInterest::isEditable() const
{
    return _editable & _isEnabled;
}

bool PointOfInterest::isVisible() const
{
    return _visible;
}

double PointOfInterest::latitude()
{
    return _latLon.latitude();
}

double PointOfInterest::longitude()
{
    return _latLon.longitude();
}

double PointOfInterest::altitude()
{
    return _altitude;
}

double PointOfInterest::azimuth()
{
    return _azimuth;
}

double PointOfInterest::elevation()
{
    return _elevation;
}

void PointOfInterest::setLatitude(double lat)
{
    latLon().setLatitude(lat);
    emit latitudeChanged();
    emit latLonChanged();
}

void PointOfInterest::setLongitude(double lon)
{
    latLon().setLongitude(lon);
    emit longitudeChanged();
    emit latLonChanged();
}

void PointOfInterest::setAltitude(double alt)
{
    _altitude = alt;
    emit altitudeChanged();
}

void PointOfInterest::setAzimuth(double azimuth)
{
    _azimuth = azimuth;
    emit azimuthChanged();
    emit pixmapChanged();
}

void PointOfInterest::setElevation(double elevation)
{
    _elevation = elevation;
    emit elevationChanged();
}

void PointOfInterest::setUseAzimuth(bool use)
{
    _useAzimuth = use;
    emit useAzimuthChanged();
}

void PointOfInterest::setUseElevation(bool use)
{
    _useAngle = use;
    emit useElevationChanged();
}

void PointOfInterest::setInfo(const QString& info)
{
    _info = info;
    infoChanged();
}

void PointOfInterest::setEditable(bool editable)
{
    _editable = !editable;
    emit editableChanged();
}

void PointOfInterest::setVisible(bool visible)
{
    _visible = visible;
    emit visibleChanged();
}

void PointOfInterest::setUseBaseColor(bool use)
{
    _useBaseColor = use;
}

const mccgeo::LatLon& PointOfInterest::latLon() const
{
    return _latLon;
}

mccgeo::LatLon& PointOfInterest::latLon()
{
    return _latLon;
}

void PointOfInterest::setLatLon(const mccgeo::LatLon& latLon)
{
    _latLon = latLon;
    emit latLonChanged();
    _updatePending = false;
    emit pixmapChanged();
}

Q_INVOKABLE void PointOfInterest::updateFinished()
{
    _updatePending = false;
    emit pixmapChanged();
}

void PointOfInterest::startMoving()
{
    _isMoving = true;
}

void PointOfInterest::finishMoving()
{
    _isMoving = false;
}

bool PointOfInterest::isMoving() const
{
    return _isMoving;
}

bool PointOfInterest::updatePending() const
{
    return _updatePending;
}

void PointOfInterest::setEnabled(bool isEnabled)
{
    _isEnabled = isEnabled;
    emit pixmapChanged();
}
}
