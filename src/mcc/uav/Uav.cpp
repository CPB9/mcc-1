#include "mcc/uav/Uav.h"
#include "mcc/ui/CoordinateSystemController.h"
#include "mcc/ui/Settings.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/UavExecCommands.h"
#include "mcc/uav/UavErrors.h"
#include "mcc/uav/UavErrorsFilteredModel.h"

#include "mcc/uav/Route.h"
#include "mcc/uav/ExchangeService.h"
#include "mcc/uav/MotionExtension.h"

#include "mcc/msg/ptr/Tm.h"
#include "mcc/msg/ParamList.h"
#include "mcc/msg/TmView.h"
#include "mcc/msg/ProtocolController.h"
#include "mcc/msg/exts/Position.h"
#include "mcc/msg/exts/Attitude.h"
#include "mcc/msg/exts/Velocity.h"

#include <bmcl/DoubleEq.h>

#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QStringList>
#include <QStyle>
#include <algorithm>
#include <bmcl/Logging.h>
#include <random>
#include <set>

#define SET_IF_CHANGED_DOUBLE(oldValue, newValue) \
    if (!bmcl::doubleEq(oldValue, newValue)) \
            { \
        oldValue = newValue; \
            };


#define SET_IF_CHANGED_DOUBLE_AND_EMIT(oldValue, newValue, signal) \
    if (!bmcl::doubleEq(oldValue, newValue)) \
    { \
        oldValue = newValue; \
        emit signal(oldValue); \
    };

#define SET_IF_CHANGED(oldValue, newValue) \
    if (oldValue != newValue) \
                    { \
        oldValue = newValue; \
                    };


#define SET_IF_CHANGED_AND_EMIT(oldValue, newValue, signal) \
    if (oldValue != newValue) \
            { \
        oldValue = newValue; \
        emit signal(oldValue); \
            };

namespace mccuav {

Uav::Uav(UavController* manager, const mccmsg::DeviceDescription& id)
    : _manager(manager)
    , _rcvdPackets(0)
    , _isAlive(false)
    , _isActivated(false)
    , _selectedRouteId(-1)
    , _targetHeading(0.0)
    , _deviceDescription(id)
{
    //setColor(Qt::yellow);

    setTrackSettings(TrackMode::Distance, 100, 1000);

    _execCommands = new UavExecCommands();
    _errors = new UavErrors();
    _filteredErrors = new UavErrorsFilteredModel();
    _filteredErrors->setSourceModel(_errors);

    _motionExtension = new MotionExtension(this);

    connect(this, &Uav::showAlert, this, [this](const QString& message)
            {
                _manager->showUavAlert(this, message);
            }
    );
}

Uav::~Uav()
{
    saveSettings();
    for (Route* route : _routes) {
        delete route;
    }
    delete _execCommands;
}

const mccmsg::Device& Uav::device() const
{
    return _deviceDescription->protocolId().device();
}

//const QString& FlyingDevice::name() const
//{
//    return _name;
//}

const QPixmap& Uav::pixmap() const
{
    return _pixmap;
}

QPixmap Uav::pixmap(float scale) const
{
    return _iconGenerator.generate(_color, scale);
}

const UavIconGenerator& Uav::pixmapGenerator() const
{
    return _iconGenerator;
}

const QPixmap& Uav::previewPixmap() const
{
    return _previewPixmap;
}

const QColor& Uav::color() const
{
    return _color;
}

Route* Uav::activeRoute()
{
    if (_activeRouteName.isNone())
        return nullptr;

    return findRouteById(_activeRouteName.unwrap());
}

Route* Uav::selectedRoute()
{
    return findRouteById(_selectedRouteId);
}

void Uav::setSelectedRoute(int routeId)
{
    if(_selectedRouteId == routeId)
        return;

    _selectedRouteId = routeId;

    auto route = selectedRoute();

    if (route == nullptr)
    {
        for (auto r : _routes)
        {
            r->setState(RouteState::Inactive);
        }
    }
    emit routeSelectionChanged(route);
    if(route)
        qDebug() << "routeSelectionChanged" << route->id();
}

void Uav::selectRoute(Route* route)
{
    int index = route == nullptr ? -1 : route->id();
    setSelectedRoute(index);
}

void Uav::resetEditableRoute(int routeIdx)
{
    auto srcRoute = std::find_if(_routes.begin(), _routes.end(), [=](Route* r) { return r->id() == routeIdx; });

    if (srcRoute == _routes.end())
    {
        Q_ASSERT(false);
        return;
    }
    (*srcRoute)->resetBuffer();
}

void Uav::resetEditableRoute(Route* route)
{
    int index = route == nullptr ? -1 : route->id();
    resetEditableRoute(index);
}

void Uav::uploadEditableRoute()
{
    auto srcRoute = std::find(_routes.begin(), _routes.end(), selectedRoute());
    if (srcRoute == _routes.end())
    {
        BMCL_WARNING() << "Неизвестный маршрут";
        return;
    }

    assert(srcRoute != nullptr);

    Route* newRoute = (*srcRoute)->buffer();
    if (newRoute == nullptr)
    {
        BMCL_WARNING() << "Буфер не задан";
        return;
    }

    mccmsg::BitFlags fs;
    fs.set(mccmsg::RouteFlag::Closed, newRoute->isLoop());
    fs.set(mccmsg::RouteFlag::Inverted, newRoute->isInverted());
    fs.set(mccmsg::RouteFlag::PointsOnly, newRoute->showPointsOnly());

    const auto& waypoints = newRoute->waypointsList();
    mccmsg::RouteProperties routeProps(waypoints.size(), newRoute->id(), "", fs, newRoute->activePointIndex());
    _manager->sendCmdYY(new mccmsg::CmdRouteSet(device(), mccmsg::Route(waypoints, routeProps)));

    emit routeUploaded();
}

void Uav::saveSettings()
{
    QJsonObject routesObj;
    for (auto r : _settings._routes)
    {
        routesObj[QString::number(r.first)] = r.second._color.name();
    }

    QJsonObject trackObj;
    trackObj["mode"]    = static_cast<int>(_trackSettings.mode());
    trackObj["seconds"] = _trackSettings.seconds();
    trackObj["meters"]  = _trackSettings.meters();

    QJsonObject settingsObj;
    settingsObj["routes"] = routesObj;
    settingsObj["track"] = trackObj;

    QJsonDocument doc(settingsObj);
    QString cfg = doc.toJson();
    emit _manager->requestUavUpdate(device(), bmcl::None, bmcl::None, bmcl::None, cfg.toStdString());
}

void Uav::setTrackSettings(mccuav::TrackMode mode, const bmcl::Option<int>& seconds, const bmcl::Option<int>& meters)
{
    if(seconds.isSome() && meters.isSome() && _trackSettings.mode() == mode && _trackSettings.seconds() == seconds.unwrap() && _trackSettings.meters() == meters.unwrap())
        return;
    _trackSettings.setMode(mode);
    if(seconds.isSome())
        _trackSettings.setSeconds(*seconds);
    if(meters.isSome())
        _trackSettings.setMeters(*meters);
    emit trackSettingsChanged();
}

const mccuav::TrackSettings& Uav::trackSettings() const
{
    return _trackSettings;
}

const mccmsg::StatDevice& Uav::statDevice() const
{
    return _statistics;
}

void Uav::setLastMsgTime(const QDateTime &time)
{
    _lastTmMsgDateTime = time;
}

const bmcl::Option<const mccgeo::Position&> Uav::position() const
{
    if(_tmStorage.isNull())
        return bmcl::None;
    auto ext = tmStorage()->getExtension<mccmsg::TmPosition>();
    if(ext.isNone())
        return bmcl::None;

    if(ext->position().isNone())
        return bmcl::None;

    return ext->position().unwrap();
}

const bmcl::Option<const mccgeo::Attitude&> Uav::attitude() const
{
    if(_tmStorage.isNull())
        return bmcl::None;
    auto ext = tmStorage()->getExtension<mccmsg::TmAttitude>();
    if(ext.isNone())
        return bmcl::None;
    if(ext->attitude().isNone())
        return bmcl::None;
    return ext->attitude().unwrap();
}

const bmcl::Option<double> Uav::speed() const
{
    if(_tmStorage.isNull())
        return bmcl::None;
    auto ext = tmStorage()->getExtension<mccmsg::TmVelocity>();
    if(ext.isNone())
        return bmcl::None;
    if(ext->speed().isNone())
        return bmcl::None;
    return ext->speed().unwrap();
}

const bmcl::Option<const mccgeo::Position&> Uav::velocity() const
{
    if(_tmStorage.isNull())
        return bmcl::None;
    auto ext = tmStorage()->getExtension<mccmsg::TmVelocity>();
    if(ext.isNone())
        return bmcl::None;
    if(ext->velocity().isNone())
        return bmcl::None;
    return ext->velocity().unwrap();
}

mccuav::UavController* Uav::uavController() const
{
    return _manager;
}

void Uav::processRouteState(const mccmsg::TmRoutePtr& routeState)
{
    Route* route = findRouteById(routeState->route().properties.name);
    if (!route)
    {
        const mccmsg::RouteProperties& props = routeState->route().properties;
        route = new Route(this, QString::fromStdString(props.info), props.name, props.maxWaypoints);
        addRoute(route);
        route->update(routeState->route());
        return;
    }

    route->update(routeState->route());
}

void Uav::processRoutesList(const mccmsg::TmRoutesListPtr& routesList)
{
    std::vector<Route*> missingRoutes;
    for (const auto& route : _routes)
    {
        const auto& rps = routesList->properties();
        const auto it = std::find_if(rps.begin(), rps.end(), [&route](const mccmsg::RouteProperties& r) { return r.name == mccmsg::RouteName(route->id()); });
        if (it == rps.end())
            missingRoutes.push_back(route);
    }

    for (const auto& it : missingRoutes)
    {
        Route* r = it;
        _routes.removeOne(it);
        emit routeRemoved(r);
        delete r;
    }

    for (const auto& props : routesList->properties())
    {
        auto route = findRouteById(props.name);
        if (!route)
        {
            addRoute(new Route(this, QString::fromStdString(props.info), props.name, props.maxWaypoints));
        }
    }

    if (!(_activeRouteName == routesList->activeRoute()))
    {
        _activeRouteName = routesList->activeRoute();
        Route* route = nullptr;
        if (_activeRouteName.isSome())
        {
            route = findRouteById(_activeRouteName.unwrap());
            assert(route != nullptr);
            if(route)
                route->setUserVisibility(true);
        }
        emit activeRouteChanged(route);
    }

    for (const auto& route : _routes)
    {
        QPen pen = route->pen();
        if (route == activeRoute() && route->state() != RouteState::Selected && route->state() != RouteState::UnderEdit)
        {
            route->setState(RouteState::Active);
        }
        else if (route->state() != RouteState::Selected && route->state() != RouteState::UnderEdit)
        {
            route->setState(RouteState::Inactive);
        }
    }
}

void Uav::processActivated(bool activated)
{
    if (_isActivated == activated)
        return;

    _isActivated = activated;

    if (_isActivated)
        setSignalGood();
    else
        setSignalBad();

    emit activatedChanged();
}


void Uav::processSetTmView(const bmcl::Rc<const mccmsg::ITmView>& view)
{
    if (_tmStorage.isNull())
    {
        auto tmStorage = _manager->protocolController()->createStorage(_deviceDescription->protocolId().protocol(), view.get());
        if (tmStorage.isNone())
        {
            assert(false);
            return;
        }
        _tmStorage = tmStorage.unwrap();
    }
    else
    {
        _tmStorage->set(view.get());
    }

    _errors->setTmStorage(_tmStorage);
    emit tmStorageUpdated();
    emit _manager->uavTmStorageUpdated(this);
    if(_tmStorage->getExtension<mccmsg::TmPosition>().isSome())
    {
        auto handler = [this]() {positionChanged(); };
        _tmStorage->getExtension<mccmsg::TmPosition>()->addHandler(std::move(handler), true).takeId();
    }
    else
        emit positionChanged();

    if(_tmStorage->getExtension<mccmsg::TmAttitude>().isSome())
    {
        auto handler = [this]() {orientationChanged(); };
        _tmStorage->getExtension<mccmsg::TmAttitude>()->addHandler(std::move(handler), true).takeId();
    }
    else
        emit orientationChanged();
}

void Uav::processUpdateTmStatusView(const bmcl::Rc<const mccmsg::ITmViewUpdate>& update)
{
    if (_tmStorage.isNull())
        return;

    _tmStorage->update(update.get());
    emit tmViewUpdated(update);
}

QTime Uav::lastTmMsgTime() const
{
    return  _lastTmMsgDateTime.time();
}

const QDateTime& Uav::lastTmMsgDateTime() const
{
    return  _lastTmMsgDateTime;
}

bool Uav::isAlive() const
{
    return _isAlive;
}

void Uav::setAlive(bool active)
{
    _isAlive = active;
}

void Uav::setSignalGood()
{
    for (auto it : _pointsOfInterest)
        it->setEnabled(true);
    emit signalGood();
}

void Uav::setSignalBad()
{
    for (auto it : _pointsOfInterest)
        it->setEnabled(false);
    emit signalBad();

    showAlert("Потеряна связь!");
}

void Uav::setStatDevice(const mccmsg::StatDevice& state)
{
    _statistics = state;

    if (state._rcvd._packets > _rcvdPackets)
    {
        setLastMsgTime(QDateTime::fromMSecsSinceEpoch(bmcl::toMsecs(state._rcvd._time.time_since_epoch()).count()));
        _rcvdPackets = state._rcvd._packets;
    }

    emit uavStatisticsChanged();
}

const mccmsg::Channels& Uav::channels() const
{
    return _channels;
}

void Uav::setChannels(const mccmsg::Channels& channels)
{
    if (_channels == channels)
        return;
    _channels = channels;
    emit channelsChanged();
}

void Uav::setFirmwareDescription(const mccmsg::FirmwareDescription& frm)
{
    _frmDescription = frm;
}

const mccmsg::DeviceDescription& Uav::deviceDescription() const
{
    return _deviceDescription;
}

void Uav::setDeviceDescription(const mccmsg::DeviceDescription& deviceDescription)
{
    _deviceDescription = deviceDescription;
    _deviceNameCached = QString::fromStdString(_deviceDescription->getName());
    _deviceInfoCached = QString::fromStdString(_deviceDescription->info());

    emit deviceDescriptionUpdated(deviceDescription);

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(QString::fromStdString(deviceDescription->settings()).toLocal8Bit(), &error);
    if (error.error == QJsonParseError::NoError)
    {
        _settings._routes.clear();

        auto settingsObj = doc.object();
        if (settingsObj.contains("routes"))
        {
            auto routesObj = settingsObj["routes"].toObject().toVariantMap();
            for (auto routeKey : routesObj.keys())
            {
                RouteSettings cfg;
                cfg._color = QColor(routesObj.value(routeKey).toString());
                _settings._routes[routeKey.toInt()] = cfg;
            }
        }

        if (settingsObj.contains("track"))
        {
            auto trackObj = settingsObj["track"].toObject();

            auto mode = trackObj.value("mode");
            auto seconds = trackObj.value("seconds");
            auto meters = trackObj.value("meters");

            setTrackSettings(static_cast<mccuav::TrackMode>(mode.toInt(0)), seconds.toInt(100), meters.toInt(10000));
        }
    }
}

const mccmsg::Protocol& Uav::protocol() const
{
    return _deviceDescription->protocolId().protocol();
}

mccmsg::DeviceId Uav::deviceId() const
{
    return _deviceDescription->protocolId().id();
}

const QString& Uav::getName() const
{
    return _deviceNameCached;
}

const QString& Uav::getInfo() const
{
    return _deviceInfoCached;
}

const bmcl::OptionRc<const mccmsg::FirmwareDescriptionObj>& Uav::firmwareDescription() const
{
    return _frmDescription;
}

const QVector<Route*>& Uav::routes() const
{
    return _routes;
}

void Uav::addRoute(Route* route)
{
    route->setMotionLimits(_motionLimits);
    _routes.append(route);
    auto it = _settings._routes.find(route->id());
    if(it != _settings._routes.end())
    {
        route->setColor(it->second._color);
    }
    else
    {
        route->setColor(_manager->nextRouteColor());
        RouteSettings routeCfg;
        routeCfg._color = route->pen().color();
        _settings._routes[route->id()] = routeCfg;
        saveSettings();
    }

    emit routeAdded(route);

    connect(route, &Route::styleChagned, this,
        [this, route](const RouteStylePtr& ptr)
        {
            _settings._routes[route->id()]._color = ptr->active.linePen.color();
            saveSettings();
        }
    );
}

Route* Uav::findRouteById(int id) const
{
    auto result = std::find_if(_routes.begin(), _routes.end(), [&](const Route* route) { return route->id() == id; });

    if (result == _routes.end())
        return nullptr;

    return *result;
}

bmcl::OptionPtr<Route> Uav::findRoute(mccmsg::RouteName name) const
{
    auto it = std::find_if(_routes.begin(), _routes.end(), [&](const Route* route) { return route->id() == static_cast<int>(name); });
    if (it == _routes.end())
        return bmcl::None;
    return *it;
}

void Uav::setActiveRoute(Route* route)
{
    int idx = _routes.indexOf(route);
    if (idx == -1)
    {
        Q_ASSERT(false);
        qDebug() << "Неверный маршрут";

        return;
    }

    _manager->sendCmdYY(new mccmsg::CmdRouteSetActive(device(), route->id(), bmcl::None));
}

void Uav::setActiveRouteAndPoint(Route* route, int index)
{
    int idx = _routes.indexOf(route);
    if (idx == -1)
    {
        Q_ASSERT(false);
        qDebug() << "Неверный маршрут";

        return;
    }
    if(index >= route->waypointsCount())
        return;

    _manager->sendCmdYY(new mccmsg::CmdRouteSetActive(device(), route->id(), index));
}

void Uav::setEmptyActiveRoute()
{
    _manager->sendCmdYY(new mccmsg::CmdRouteSetNoActive(device()));
}

void Uav::changeRouteDirection(Route* route, bool forward)
{
    if(route == nullptr)
        return;
    _manager->sendCmdYY(new mccmsg::CmdRouteSetDirection(device(), route->id(), forward));
}

void Uav::setNextWaypoint(Route* route, int index)
{
    if(route == nullptr)
        return;
    _manager->sendCmdYY(new mccmsg::CmdRouteSetActivePoint(device(), route->id(), index));
}

void Uav::clearNextWaypoint(Route* route)
{
    if(route == nullptr)
        return;
    _manager->sendCmdYY(new mccmsg::CmdRouteSetActivePoint(device(), route->id(), bmcl::Option<uint32_t>()));
}

void Uav::setMotionLimits(const UavMotionLimits& limits)
{
    _motionLimits = limits;
    for (auto& r : _routes)
    {
        r->setMotionLimits(_motionLimits);
    }

    emit motionLimitsChanged();
}

const UavMotionLimits& Uav::motionLimits() const
{
    return _motionLimits;
}

void Uav::createRoute(mccmsg::RouteName name)
{
    _manager->sendCmdYY(new mccmsg::CmdRouteCreate(device(), name));
}

void Uav::removeRoute(mccmsg::RouteName name)
{
    _manager->sendCmdYY(new mccmsg::CmdRouteRemove(device(), name));
}

bmcl::Option<mccmsg::Group> Uav::group() const
{
    return bmcl::None;
}

bmcl::Option<mccmsg::GroupId> Uav::groupId() const
{
    return bmcl::None;
//     if (_state.isNone() || _state->group.isNone())
//         return bmcl::None;
//     return mccmsg::groupToId(_state->group.unwrap());
}

int Uav::qmlGroupId() const
{
    return -1;
//     if (_state.isNone() || _state->group.isNone())
//         return -1;
//     return mccmsg::groupToId(_state->group.unwrap());
}

const PointOfInterestPtrs& Uav::pointsOfInterest() const
{
    return _pointsOfInterest;
}

void Uav::addPointOfInterest(const PointOfInterestPtr& point)
{
    _pointsOfInterest.push_back(point);
    point->setBaseColor(_color);
    emit pointOfInterestAdded(point);
}

void Uav::removePointOfInterest(const PointOfInterestPtr& point)
{
    auto it = std::find(_pointsOfInterest.begin(), _pointsOfInterest.end(), point);
    if (it == _pointsOfInterest.end())
    {
        Q_ASSERT(false);
        return;
    }

    emit pointOfInterestRemoved(point);
    _pointsOfInterest.erase(it);
}

void Uav::clearPointsOfInterest()
{
    for (auto it : _pointsOfInterest)
        emit pointOfInterestRemoved(it);
    _pointsOfInterest.clear();
}

const mccuav::UserParamPtrs& Uav::userParams() const
{
    return _userParams;
}

void Uav::addUserParam(const UserParamPtr& point)
{
    _userParams.push_back(point);
    emit userParamAdded(point);
}

void Uav::removeUserParam(const UserParamPtr& point)
{
    auto it = std::find(_userParams.begin(), _userParams.end(), point);
    if(it == _userParams.end())
    {
        Q_ASSERT(false);
        return;
    }

    emit userParamRemoved(point);
    _userParams.erase(it);
}

void Uav::clearUserParams()
{
    for(auto it : _userParams)
        emit userParamRemoved(it);
    _userParams.clear();
}

double Uav::distanceToWaypoint(const mccmsg::Waypoint& wp, double* direction)
{
    double distance = 0.0;
    if (position().isNone())
        return distance;
    _manager->geod().inverse(position()->latLon(), wp.position.latLon(), &distance, 0, direction);
    return distance;
}

mccuav::MotionExtension* Uav::motionExtension()
{
    return _motionExtension;
}

QString Uav::getDeviceDescriptionText() const
{
    return QString::number(_deviceDescription->protocolId().id());
}

bool Uav::hasFeature(UavFeatures feature) const
{
    return false;
}

void Uav::onCmdAdded(const mccmsg::DevReqPtr& cmd)
{
    if (cmd->device() != this->device())
        return;

    _execCommands->addCommand(cmd);
}

void Uav::onCmdStateChanged(const mccmsg::DevReqPtr& req, const mccmsg::Request_StatePtr& state)
{
    _execCommands->updateCommand(req, state);
}

void Uav::onCmdRemoved(const mccmsg::DevReqPtr& req)
{
    if (req->device() != this->device())
        return;

    _execCommands->removeCommand(req);
}


UavExecCommands* Uav::execCommands() const
{
    return _execCommands;
}

mccuav::UavErrors* Uav::errors()
{
    return _errors;
}

const mccuav::UavErrorsFilteredModel* Uav::filteredErrors() const
{
    return _filteredErrors;
}

mccuav::UavErrorsFilteredModel* Uav::filteredErrors()
{
    return _filteredErrors;
}

const bmcl::Rc<mccmsg::ITmStorage>& Uav::tmStorage() const
{
    return _tmStorage;
}

bool Uav::isRegistered() const
{
    return _frmDescription.isSome();
}

bmcl::Option<std::size_t> Uav::nextWaypoint() const
{
    if (_activeRouteName.isNone())
        return bmcl::None;

    Route* selectedR = findRouteById(_activeRouteName.unwrap());
    if(selectedR != nullptr)
        return selectedR->activePointIndex();

    return bmcl::None;
}

void Uav::setSourcePixmapFile(bmcl::Bytes pixmap)
{
    _sourcePixmap = QString::fromUtf8((const char*)pixmap.data(), pixmap.size());
    _iconGenerator.setContents(_sourcePixmap);
    setColor(_color, _manager->uavPixmapScale());
}

void Uav::setColor(const QColor& color, double scale)
{
    _color = color;

    _pixmap = _iconGenerator.generate(color, scale);
    _previewPixmap = _iconGenerator.generate(color, 40, 40);
    emit pixmapChanged(_pixmap);

    for(auto& p : _pointsOfInterest)
        p->setBaseColor(_color);
}

void Uav::setColor(const QColor& color)
{
    setColor(color, _manager->uavPixmapScale());
}

bool Uav::isStateActive()
{
    return _statistics._isActive;
}

bool Uav::isActivated() const
{
    return _isActivated;
}

}
