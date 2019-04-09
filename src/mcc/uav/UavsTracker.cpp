#include "mcc/uav/UavsTracker.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/Uav.h"
#include "mcc/ui/Settings.h"
#include "mcc/ui/CoordinateSystemController.h"

namespace mccuav {

constexpr QChar degreeChar(0260);

UavsTracker::UavsTracker(UavController* controller, mccui::Settings* settings, QObject *parent /*= nullptr*/)
    : _uavController(controller)
{
    //_enabled = settings->showAirTower();
    _enabled = true;

    _watcherLocation.emplace();
    QString latKey = "gps/tower/latitude";
    QString lonKey = "gps/tower/longitude";
    _watcherLocation->latitude() = settings->read(latKey, 0).toDouble();
    _watcherLocation->longitude() = settings->read(lonKey, 0).toDouble();

    settings->onChange(latKey, this, [this](const QVariant& value) {
        _watcherLocation->latitude() = value.toDouble();
        update();
    });
    settings->onChange(lonKey, this, [this](const QVariant& value) {
        _watcherLocation->longitude() = value.toDouble();
        update();
    });

    connect(controller, &UavController::uavAdded, this, &UavsTracker::addUav);
    connect(controller, &UavController::uavRemoved, this, &UavsTracker::removeUav);

    startTimer(1000);
}

UavsTracker::~UavsTracker()
{

}

bool UavsTracker::enabled() const
{
    return _enabled;
}

void UavsTracker::setEnabled(bool enabled)
{
    _enabled = true;
    update();
}

bmcl::Option<mccuav::UavTrackLocation> UavsTracker::trackUav(Uav* uav) const
{
    if (_watcherLocation.isNone())
        return bmcl::None;

    auto it = _uavsLocation.find(uav);
    if (it == _uavsLocation.end())
        return bmcl::None;
    return it->second;
}

void UavsTracker::setWatchLocation(const mccgeo::LatLon& latLon)
{
    _watcherLocation = latLon;
    update();
}

void UavsTracker::update()
{
    if (_watcherLocation.isNone() || !_enabled)
        return;

    for (auto& uavAndLoc : _uavsLocation)
    {
        const auto& motion = uavAndLoc.first->position();
        if (motion.isNone())
        {
            uavAndLoc.second.clear();
            emit uavTrackUpdated(uavAndLoc.first, bmcl::None);
            return;
        }

        if (uavAndLoc.second.isNone())
            uavAndLoc.second.emplace();

        _uavController->geod().inverse(motion->latLon(), _watcherLocation.unwrap(), &uavAndLoc.second->distance, 0, &uavAndLoc.second->azimuth);
        uavAndLoc.second->azimuth += 180;
        emit uavTrackUpdated(uavAndLoc.first, uavAndLoc.second);
    }
}

void UavsTracker::addUav(Uav* uav)
{
    auto it = _uavsLocation.find(uav);
    if (it == _uavsLocation.end())
        _uavsLocation.insert(std::make_pair(uav, bmcl::None));
}

void UavsTracker::removeUav(Uav* uav)
{
    auto it = _uavsLocation.find(uav);
    if (it != _uavsLocation.end())
        _uavsLocation.erase(uav);
}

void UavsTracker::timerEvent(QTimerEvent *event)
{
    update();
}

UavsTrackerPluginData::UavsTrackerPluginData(UavsTracker* tracker)
    : mccplugin::PluginData(id)
    , _uavsTracker(tracker)
{
}

UavsTrackerPluginData::~UavsTrackerPluginData()
{

}

mccuav::UavsTracker* UavsTrackerPluginData::uavsTracker()
{
    return _uavsTracker.get();
}

const mccuav::UavsTracker* UavsTrackerPluginData::uavsTracker() const
{
    return _uavsTracker.get();
}

QString UavTrackLocation::toQString() const
{
    return QString("%1%2, %3км").arg(azimuth, 3, 'f', 0)
        .arg(degreeChar)
        .arg(distance / 1000, 3, 'f', 2);
}

}
