#include "mcc/uav/Route.h"
#include "mcc/geo/Bbox.h"
#include "mcc/msg/Route.h"
#include "mcc/msg/ProtocolController.h"
#include <QColor>
#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>
#include <QJsonParseError>
#include <bmcl/Logging.h>
#include <bmcl/DoubleEq.h>

namespace mccuav {

static const char* kmlLineStringTemplate =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n\
<Document>\n\
<name>${MISSION_NAME}</name>\n\
<description>${MISSION_DESCRIPTION}</description>\n\
<Style id=\"yellowLineGreenPoly\">\n\
    <LineStyle>\n\
        <color>${ROUTE_COLOR}</color>\n\
        <width>4</width>\n\
    </LineStyle>\n\
    <PolyStyle>\n\
        <color>7f00ff00</color>\n\
    </PolyStyle>\n\
</Style>\n\
<Placemark>\n\
    <name>${ROUTE_NAME}</name>\n\
    <description>${ROUTE_DESCRIPTION}</description>\n\
    <styleUrl>#yellowLineGreenPoly</styleUrl>\n\
    <LineString>\n\
        <extrude>1</extrude>\n\
        <tessellate>1</tessellate>\n\
        <altitudeMode>absolute</altitudeMode>\n\
        <coordinates> ${ROUTE_POINTS}\n\
        </coordinates>\n\
    </LineString>\n\
</Placemark>\n\
</Document>\n\
</kml>";

bool operator ==(const Route::WaypointHint& left, const Route::WaypointHint& right)
{
    return bmcl::doubleEq(left.distance, right.distance) && left.indexAfter == right.indexAfter && left.latLon == right.latLon;
}

Route::Route(const QString& name, int id, size_t maxCount, bool isBuffer)
    : QObject()
    , _isRing(true)
    , _showDetails(true)
    , _isSelected(false)
    , _id(id)
    , _maxCount(static_cast<int>(maxCount))
    , _isVisible(true)
    , _isReadOnly(false)
    , _isInverse(false)
    , _showPointsOnly(false)
    , _activePointIndex(bmcl::None)
    , _isEnabled(true)
    , _isHidden(true)
    , _name(name)
    , _buffer(nullptr)
{
    _style = std::make_shared<RouteStyle>();
    _style->active.linePen.setColor(Qt::yellow);
    _style->active.linePen.setWidth(3);
    _style->active.activePointColor = Qt::red;
    _style->active.inactivePointColor = Qt::blue;
    _style->active.hoverColor = Qt::yellow;
    _style->active.hasDetails = true;
    _style->active.selectionColor = Qt::blue;

    _style->editable.linePen.setWidth(3);
    _style->editable.linePen.setStyle(Qt::PenStyle::DashLine);
    _style->editable.linePen.setColor(Qt::red);
    _style->editable.activePointColor = Qt::red;
    _style->editable.inactivePointColor = Qt::blue;
    _style->editable.hoverColor = Qt::yellow;
    _style->editable.hasDetails = true;
    _style->editable.selectionColor = QColor("#0DE6DF");

    _style->inactive.linePen.setColor(Qt::gray);
    _style->inactive.activePointColor = Qt::gray;
    _style->inactive.inactivePointColor = Qt::gray;
    _style->inactive.selectionColor = Qt::gray;

    _style->inactive.hasDetails = false;

    if (!isBuffer)
        _buffer = new Route(name + "_buffer", id, _maxCount, true);
}

Route::~Route()
{
    delete _buffer;
}

void Route::setMotionLimits(const UavMotionLimits& limits)
{
    _motionsLimits = limits;
    if (buffer())
        buffer()->setMotionLimits(limits);
}

void Route::clear()
{
    setActivePoint(bmcl::None);
    clearSelection();
    setClosedPath(false);
    setInverted(false);
    _points.clear();
    emit allWaypointsChanged();
}

const mccmsg::Waypoint& Route::waypointAt(int idx) const
{
    Q_ASSERT(idx >= 0 && idx < _points.size());

    return _points[idx];
}

mccmsg::Waypoint& Route::waypointAt(int idx)
{
    Q_ASSERT(idx >= 0 && idx < _points.size());
    return _points[idx];
}

const mccmsg::Waypoints& Route::waypointsList() const
{
    return _points;
}

bool Route::isValid() const
{
    for (size_t i = 0; i < waypointsCount(); ++i)
        if (!isPointValid(i))
            return false;
    return true;
}

void Route::addWaypointNoEmit(const mccmsg::Waypoint& waypoint)
{
    if (_points.size() >= _maxCount)
        return;

    _points.push_back(waypoint);
}

bool Route::addWaypoint(const mccmsg::Waypoint& waypoint)
{
    if (_points.size() >= _maxCount)
        return false;

    _points.push_back(waypoint);
    emit waypointInserted(waypoint, _points.size() - 1);

    syncActivePoint();
    return true;
}

bool Route::setWaypoint(const mccmsg::Waypoint& waypoint, int idx)
{
    if (idx >= _points.size())
        return false;

    if (_points[idx] != waypoint)
    {
        _points[idx] = waypoint;
        emit waypointChanged(waypoint, idx);
    }

    return true;
}

void Route::setWaipointAlt(double alt, int idx) {
    if (idx >= _points.size())
        return;
    _points[idx].position.altitude() = alt;
    emit waypointOnlyAltChanged(_points[idx], idx);
}

bool Route::insertWaypoint(const mccmsg::Waypoint& waypoint, int afterIndex /*= -1*/)
{
    if (waypointsCount() >= _maxCount)
        return false;

    if (afterIndex == -1)
        afterIndex = _points.size();

    _points.insert(_points.begin() + afterIndex, waypoint);

    emit waypointInserted(waypoint, afterIndex);

    setSelectedPoint(afterIndex);
    syncActivePoint();

    return true;
}

bool Route::canInsertWaypoint(int afterIndex)
{
    (void)afterIndex;
    return waypointsCount() < _maxCount;
}

bool Route::removeWaypoint(int index)
{
    if (index < 0 || index >= waypointsCount())
        return false;

    _points.erase(_points.begin() + index);
    emit waypointRemoved(index);

    syncActivePoint();
    syncSelectedPoint();

    return true;
}

bool Route::removeSelectedWaypoints()
{
    if(_selectedPointIndexes.empty())
        return false;

    bool ok = true;
    for (auto i : _selectedPointIndexes)
    {
        ok &= removeWaypoint(i);
    }
    return ok;
}

bool Route::setActivePoint(const bmcl::Option<std::size_t>& index)
{
    _activePointIndex = activePointIndex();
    if (_activePointIndex == index)
        return false;

    for (auto& wp : _points)
        wp.isActive = false;

    if (index.isSome())
    {
        int idx = static_cast<int>(index.unwrap());

        if (idx >= waypointsCount())
        {
            //qDebug() << "idx < 0 || idx >= waypointsCount(). RouteId: " << _name << " idx == " << idx << " waypointsCount: " << waypointsCount();
            _activePointIndex = bmcl::None;
            emit activeWaypointChanged(_activePointIndex);
            return false;
        }

        _points[idx].isActive = true;
    }

    _activePointIndex = index;
    emit activeWaypointChanged(index);

    return true;
}

void Route::selectAll()
{
    _selectedPointIndexes.clear();
    _selectedPointIndexes.reserve(_points.size());
    for (std::size_t i = 0; i < _points.size(); i++) {
        _selectedPointIndexes.push_back(i);
        emit selectedWaypointsChanged(i, true);
    }
}

void Route::clearSelection()
{
    if (_selectedPointIndexes.empty())
        return;
    auto indexesToEmit = _selectedPointIndexes;

    _selectedPointIndexes.clear();
    for (auto idx : indexesToEmit)
    {
        emit selectedWaypointsChanged(idx, false);
    }
}

void Route::setSelectedPoint(std::size_t index, bool forced /*= false*/)
{
    setSelectedPoints({ index }, forced);
}

void Route::selectPoint(std::size_t index)
{
    auto it = std::find(_selectedPointIndexes.begin(), _selectedPointIndexes.end(), index);
    if (it == _selectedPointIndexes.end()) {
        _selectedPointIndexes.push_back(index);
        emit selectedWaypointsChanged(index, true);
    }
}

void Route::deselectPoint(std::size_t index)
{
    auto it = std::find(_selectedPointIndexes.begin(), _selectedPointIndexes.end(), index);
    if (it != _selectedPointIndexes.end()) {
        _selectedPointIndexes.erase(it);
        emit selectedWaypointsChanged(index, false);
    }
}

void Route::setSelectedPoints(const std::vector<std::size_t>& indexes, bool forced)
{
    if (indexes.empty())
    {
        clearSelection();
        return;
    }

    bool needUpdate = (_selectedPointIndexes != indexes);
    if (forced)
        needUpdate = true;

    if (needUpdate)
    {
        clearSelection();
        _selectedPointIndexes = indexes;
        for (auto idx : _selectedPointIndexes)
        {
            emit selectedWaypointsChanged(idx, true);
        }
    }
}

void Route::setInverted(bool inverse)
{
    if (_isInverse != inverse)
    {
        _isInverse = inverse;
        emit inverseFlagChanged(inverse);
    }
}

void Route::setShowPointsOnly(bool show)
{
    if (_showPointsOnly != show)
    {
        _showPointsOnly = show;
        emit showPointsOnlyFlagChanged(show);
    }
}

bool Route::moveWaypointUp(int index)
{
    if (index <= 0)
        return false;

    int oldIndex = index;
    int newIndex = index - 1;

    auto wp = _points[oldIndex];
    _points.erase(_points.begin() + oldIndex);
    _points.insert(_points.begin() + newIndex, wp);

    emit waypointMoved(oldIndex, newIndex);
    syncActivePoint();

    return true;
}

bool Route::moveWaypointDown(int index)
{
    if (index >= _points.size() - 1)
        return false;

    int oldIndex = index;
    int newIndex = index + 1;

    auto wp = _points[oldIndex];
    _points.erase(_points.begin() + oldIndex);
    _points.insert(_points.begin() + newIndex, wp);

    emit waypointMoved(oldIndex, newIndex);
    syncActivePoint();

    return true;
}

void Route::setWaypoints(const mccmsg::Waypoints& waypoints)
{
    if (_points == waypoints)
        return;

    setActivePoint(bmcl::None);
    clearSelection();

    _points = waypoints;
    emit allWaypointsChanged();

    syncActivePoint();
    syncSelectedPoint();
}

bool Route::isLoop() const
{
    return _isRing;
}

void Route::setClosedPath(bool isRing)
{
    if (_isRing == isRing)
        return;

    _isRing = isRing;
    emit closedPathFlagChanged(isRing);
}

bool Route::isUserVisible() const
{
    return _isVisible;
}

bool Route::isReadOnly() const
{
    return _isReadOnly;
}

bool Route::isInverted() const
{
    return _isInverse;
}

bool Route::showPointsOnly() const
{
    return _showPointsOnly;
}

void Route::setUserVisibility(bool visible)
{
    if (_isVisible != visible)
    {
        _isVisible = visible;
        emit userVisibilityFlagChanged(_isVisible);
    }
}


void Route::setHidden(bool hidden)
{
    if (_isHidden != hidden)
    {
        _isHidden = hidden;
        emit hiddenFlagChanged(_isHidden);
    }
}

void Route::setReadOnly(bool readOnly)
{
    if (_isReadOnly != readOnly)
    {
        _isReadOnly = readOnly;
        emit readOnlyFlagChanged(_isReadOnly);
    }
}

void Route::setState(RouteState state)
{
    if (_style->_state == state)
        return;

    _style->setState(state);
    emit styleChagned(_style);
}

void Route::setColor(const QColor& color)
{
    _style->setLineColor(color);
    emit styleChagned(_style);
}

void Route::setWaypointHint(const bmcl::Option<WaypointHint>& hint)
{
    if (hint == _waypointHint)
        return;
    _waypointHint = hint;
    emit waypointHintChanged();
}

void Route::requestEditDialog(int index, Route::EditMode mode)
{
    emit showEditDialog(index, mode);
}

const QPen& Route::pen() const
{
    return _style->active.linePen;
}


const QColor& Route::activePointColor() const
{
    return _style->active.activePointColor;
}

const QColor& Route::inactivePointColor() const
{
    return _style->active.inactivePointColor;
}

const QString& Route::name() const
{
    return _name;
}

int Route::id() const
{
    return _id;
}

//             void Route::setStyle(const QPen& pen, const QColor& activePointColor, const QColor& inactivePointColor, bool showDetails)
//             {
//                 _style->active.linePen = pen;
//                 _style->active.activePointColor = activePointColor;
//                 _style->active.inactivePointColor = inactivePointColor;
//                 _style->active.hasDetails = showDetails;
//
//                 emit styleChagned(_style);
//             }

void Route::copyFrom(Route* other)
{
    _id = other->id();
    setClosedPath(other->isLoop());
    setInverted(other->isInverted());
    setShowPointsOnly(other->showPointsOnly());
    setWaypoints(other->waypointsList());

    if (other->activePoint().isSome())
        setActivePoint(other->activePointIndex());
    else
        setActivePoint(bmcl::None);
    setSelectedPoints(other->selectedPointIndexes());
    emit activeWaypointChanged(activePointIndex());
}

mccgeo::Bbox Route::computeBoundindBox() const
{
    if (_points.size() == 0)
        return mccgeo::Bbox();
    else if (_points.size() == 1)
        return mccgeo::Bbox(_points[0].position);

    mccgeo::Bbox bbox(_points[0].position);
    for (auto it = (_points.begin() + 1); it < _points.end(); it++)
    {
        double lat = it->position.latitude();
        if (lat > bbox.left())
            bbox.setLeft(lat);
        else if (lat < bbox.right())
            bbox.setRight(lat);
        double lon = it->position.longitude();
        if (lon < bbox.top())
            bbox.setTop(lon);
        else if (lon > bbox.bottom())
            bbox.setBottom(lon);
    }
    return bbox;
}

Route* Route::buffer() const
{
    return _buffer;
}

void Route::resetBuffer()
{
    _buffer->copyFrom(this);
}

void Route::save(const QString& path, FileFormat fmt) const
{
    if(path.isEmpty())
        return;

    switch (fmt)
    {
    case FileFormat::Json:
        saveJson(path);
        return;
    case FileFormat::KmlLineString:
        saveKmlLineString(path);
        return;
    }
}

void Route::load(const QString& path, const mccmsg::ProtocolController* p)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Unable to open " << path;
        return;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);

    if (error.error != QJsonParseError::NoError)
    {
        qDebug() << "Wrong json document " << path;
        return;
    }

    QJsonObject routeObj = doc.object();

    if (!routeObj.contains("waypoints"))
    {
        qDebug() << "Wrong json document " << path;
        return;
    }

    if (!routeObj["waypoints"].isArray())
    {
        qDebug() << "Wrong json document " << path;
        return;
    }

    QJsonArray waypointsObj = routeObj["waypoints"].toArray();

    mccmsg::Waypoints wps;
    for (const auto& wp : waypointsObj)
    {
        QJsonObject wpObj = wp.toObject();

        double latitude = wpObj["latitude"].toDouble();
        double longitude = wpObj["longitude"].toDouble();
        double altitude = wpObj["altitude"].toDouble();
        double speed = wpObj["speed"].toDouble();
        double direction = wpObj["direction"].toDouble();

        mccmsg::PropertyValues flags;
        QJsonArray fs = wpObj["flags"].toArray();
        mccmsg::PropertyValues fsValues;
        for (const auto& f : fs)
        {
            assert(f.isObject());
            if (!f.isObject())
                continue;
            auto o = f.toObject();

            auto name = o["name"].toString().toStdString();
            auto value = o["value"].toString().toStdString();

            auto r = p->decodeProperty(mccmsg::Property::createOrNil(name.c_str()), value.c_str());
            if (r.isNone())
            {
                BMCL_WARNING() << "не удалось десериализовать свойство точки маршрута! " << name << " " << value;
                continue;
            }
            fsValues.add(r.take());
        }

        wps.emplace_back(mccmsg::Waypoint(mccgeo::Position(latitude, longitude, altitude), speed, direction, flags));
    }

    bool isRing = routeObj["ring"].toBool();
    bool isInverted = routeObj["inverted"].toBool();
    setWaypoints(wps);
    setClosedPath(isRing);
    setInverted(isInverted);
}

const RouteStylePtr& Route::style() const
{
    return _style;
}

RouteState Route::state() const
{
    return _style->_state;
}

QString Route::generateSaveName() const
{
    auto time = QDateTime::currentDateTime();
    return _name + time.toString("_yyyy-MM-dd_hh-mm") + filenameExtension();
}

void Route::saveJson(const QString& path) const
{
    QJsonArray waypoints;

    for (const auto& wp : _points)
    {
        QJsonObject wpObj;
        wpObj["latitude"] = wp.position.latitude();
        wpObj["longitude"] = wp.position.longitude();
        wpObj["altitude"] = wp.position.altitude();

        wpObj["speed"] = wp.speed;
        wpObj["direction"] = wp.direction;

        QJsonArray flags;
        for (const auto& f : wp.properties.values())
        {
            QJsonObject fObj;
            fObj["name"] = f->property()->name().toQString();
            fObj["info"] = QString::fromStdString(f->property()->info());
            fObj["value"] = QString::fromStdString(f->encode());
            flags.append(fObj);
        }
        wpObj["flags"] = flags;
        waypoints.append(wpObj);
    }

    QJsonObject routeObj;
    routeObj["waypoints"] = waypoints;
    routeObj["ring"] = this->isLoop();
    routeObj["inverted"] = this->isInverted();

    QFile outputFile(path);
    outputFile.open(QIODevice::WriteOnly);

    if (!outputFile.isOpen())
    {
        qDebug() << "Unable to open " << path;
        return;
    }

    QJsonDocument doc;
    doc.setObject(routeObj);

    outputFile.write(doc.toJson(QJsonDocument::Indented));
    outputFile.close();
}

void Route::saveKmlLineString(const QString& path) const
{
    QString missionName;
    QString missionDescription;
    QString routeName = _name;
    QString routeDescription;
    QColor tmp = _style->active.linePen.color();
    QString routeColor = QString("%1%2%3%4")
        .arg(QString::asprintf("%02X", tmp.alpha()))
        .arg(QString::asprintf("%02X", tmp.blue()))
        .arg(QString::asprintf("%02X", tmp.green()))
        .arg(QString::asprintf("%02X", tmp.red()));
    QString routePoints;
    for (const auto& wp : _points)
    {
        QString coordinates = QString("                %1,%2,%3\n").arg(wp.position.longitude()).arg(wp.position.latitude()).arg(wp.position.altitude());
        routePoints.append(coordinates);
    }

    QString document = kmlLineStringTemplate;
    document.replace("${ROUTE_POINTS}", routePoints);
    document.replace("${MISSION_NAME}", missionName);
    document.replace("${MISSION_DESCRIPTION}", missionDescription);
    document.replace("${ROUTE_NAME}", routeName);
    document.replace("${ROUTE_DESCRIPTION}", routeDescription);
    document.replace("${ROUTE_COLOR}", routeColor);

    QFile outputFile(path);
    outputFile.open(QIODevice::WriteOnly);

    if (!outputFile.isOpen())
    {
        qDebug() << "Unable to open " << path;
        return;
    }

    QTextStream stream(&outputFile);
    stream.setCodec("UTF-8");
    stream << document;
    outputFile.close();
}

void Route::syncActivePoint()
{
    auto realIndex = activePointIndex();
    if (_activePointIndex == realIndex)
        return;

    for (auto& wp : _points)
        wp.isActive = false;

    if (realIndex.isSome())
        _points[static_cast<int>(realIndex.unwrap())].isActive = true;
    _activePointIndex = realIndex;

    emit activeWaypointChanged(realIndex);
}

void Route::syncSelectedPoint()
{
//     bmcl::Option<std::size_t> newSelected;
//     if (_selectedPointIndexes.isSome())
//     {
//         if(!isEmpty())
//         {
//             if(_selectedPointIndexes.unwrap() >= static_cast<std::size_t>(waypointsCount()))
//                 newSelected = static_cast<std::size_t>(waypointsCount() - 1);
//             else
//                 newSelected = _selectedPointIndexes;
//         }
//     }
//     setSelectedPoint(newSelected, true);
}

void Route::reverse()
{
    std::reverse(_points.begin(), _points.end());
    emit allWaypointsChanged();
    syncActivePoint();
    syncSelectedPoint();
}

bool Route::showDetails() const
{
    return _showDetails;
}

bmcl::Option<const mccmsg::Waypoint&> Route::activePoint() const
{
    const auto it = std::find_if(_points.begin(), _points.end(), [](const mccmsg::Waypoint& wp) { return wp.isActive; });
    if (it == _points.end())
        return bmcl::None;
    return *it;
}

bmcl::Option<std::size_t> Route::activePointIndex() const
{
    auto wp = activePoint();
    if(wp.isNone())
        return bmcl::None;

    return indexOf(wp.unwrap());
}

const std::vector<std::size_t>& Route::selectedPointIndexes() const
{
    return _selectedPointIndexes;
}

bool Route::isPointSelected(size_t index) const
{
    return std::find(_selectedPointIndexes.begin(), _selectedPointIndexes.end(), index) != _selectedPointIndexes.end();
}

bool Route::isPointValid(size_t index) const
{
    if(_motionsLimits.isNone())
        return true;
    if (index >= waypointsCount())
        return false;
    const auto& wp = waypointAt(index);
    return isWaypointVelocityValid(wp) & isWaypointAltitudeValid(wp);
}

bool Route::isWaypointVelocityValid(const mccmsg::Waypoint& wp) const
{
    if (_motionsLimits.isNone())
        return true;

    bool isValid = true;
    if (_motionsLimits->minVelocity.isSome())
        isValid &= (wp.speed >= _motionsLimits->minVelocity.unwrap());
    if (_motionsLimits->maxVelocity.isSome())
        isValid &= (wp.speed <= _motionsLimits->maxVelocity.unwrap());
    return isValid;
}

bool Route::isWaypointAltitudeValid(const mccmsg::Waypoint& wp) const
{
    if (_motionsLimits.isNone())
        return true;

    bool isValid = true;
    if (_motionsLimits->minAltitude.isSome())
        isValid &= (wp.position.altitude() >= _motionsLimits->minAltitude.unwrap());
    if (_motionsLimits->maxAltitude.isSome())
        isValid &= (wp.position.altitude() <= _motionsLimits->maxAltitude.unwrap());
    return isValid;
}

size_t Route::indexOf(const mccmsg::Waypoint& wp) const
{
    const auto it = std::find(_points.begin(), _points.end(), wp);
    if (it == _points.end())
    {
        BMCL_ASSERT(false);
        return 0;
    }

    return std::distance(_points.begin(), it);
}

bool Route::isEnabled() const
{
    return _isEnabled;
}

bool Route::isHidden() const
{
    return _isHidden;
}

bmcl::Option<mccuav::Route::WaypointHint> Route::waypointHint() const
{
    return _waypointHint;
}

void Route::update(const mccmsg::Route& other)
{
    setWaypoints(other.waypoints);
    setClosedPath(other.properties.flags.get(mccmsg::RouteFlag::Closed));
    bool otherEnabled = !other.properties.flags.get(mccmsg::RouteFlag::Locked);
    if (_isEnabled != otherEnabled)
    {
        _isEnabled = otherEnabled;
        emit enabledFlagChanged(_isEnabled);

        setUserVisibility(_isEnabled);
    }
    setReadOnly(other.properties.flags.get(mccmsg::RouteFlag::ReadOnly));
    setInverted(other.properties.flags.get(mccmsg::RouteFlag::Inverted));
    setShowPointsOnly(other.properties.flags.get(mccmsg::RouteFlag::PointsOnly));

    if (other.properties.nextWaypoint.isNone())
        setActivePoint(bmcl::None);
    else
        setActivePoint(other.properties.nextWaypoint.unwrap());

    setHidden(other.properties.flags.get(mccmsg::RouteFlag::Hidden));
}
}
