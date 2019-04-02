#pragma once

#include "mcc/Config.h"
#include "mcc/uav/Rc.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/QObjectRefCountable.h"
#include "mcc/geo/LatLon.h"
#include "mcc/plugin/PluginData.h"
#include <QObject>

#include <bmcl/Option.h>

namespace mcchm { class HmReader; }

namespace mccuav {

class Uav;
class UavController;

struct MCC_UAV_DECLSPEC UavTrackLocation
{
    double azimuth;
    double distance;

    QString toQString() const;
};

class MCC_UAV_DECLSPEC UavsTracker : public mccui::QObjectRefCountable<QObject>
{
    Q_OBJECT

public:
    explicit UavsTracker(UavController* controller, mccui::Settings* settings, QObject *parent = nullptr);
    ~UavsTracker();

    bool enabled() const;
    void setEnabled(bool enabled);

    bmcl::Option<UavTrackLocation> trackUav(Uav* uav) const;
    void setWatchLocation(const mccgeo::LatLon& latLon);
signals:
    void uavTrackUpdated(mccuav::Uav* uav, bmcl::Option<UavTrackLocation> location);
private:
    void update();
private slots:
    void addUav(Uav* uav);
    void removeUav(Uav* uav);

private:
    std::map<Uav*, bmcl::Option<UavTrackLocation>> _uavsLocation;
    Rc<mccuav::UavController>           _uavController;
    bmcl::Option<mccgeo::LatLon>        _watcherLocation;
    bool                                _enabled;

protected:
    virtual void timerEvent(QTimerEvent *event) override;

};

class MCC_UAV_DECLSPEC UavsTrackerPluginData : public mccplugin::PluginData {
public:
    static constexpr const char* id = "mcc::UavsTrackerPluginData";

    UavsTrackerPluginData(UavsTracker* tracker);
    ~UavsTrackerPluginData();

    UavsTracker* uavsTracker();
    const UavsTracker* uavsTracker() const;

private:
    Rc<UavsTracker> _uavsTracker;
};
}
