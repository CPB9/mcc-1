#pragma once
#include "mcc/Config.h"
#include "mcc/uav/Rc.h"
#include "mcc/ui/QObjectRefCountable.h"
#include "mcc/geo/Fwd.h"
#include "mcc/plugin/PluginData.h"

#include <bmcl/Option.h>

#include <QObject>

namespace mccuav {

class Route;
class Uav;

class MCC_UAV_DECLSPEC RoutesController : public mccui::QObjectRefCountable<QObject>
{
    Q_OBJECT

public:
    explicit RoutesController(QObject *parent = nullptr);
    ~RoutesController() override;

    bool isEditing() const {return _isEditing;}

    size_t availableRoutesCount() const;
    Route* selectedRoute() const {return _selectedRoute;}
    Route* editableRoute() const;

    bmcl::Option<const std::vector<std::size_t>&> selectedWaypoint() const;

signals:
    void selectedRouteChanged(Route* route, Uav* uav = nullptr);
    void routeEditingChanged(bool isEditing);
    void routeCentering(const mccgeo::Bbox& bbox);
    void routeCentering(const mccgeo::LatLon& latLon);

    void selectedWaypointChanged(const std::vector<std::size_t>& index);
    void activeWaypointChanged(const bmcl::Option<std::size_t>& index);

public slots:
    void acceptUavSelection(Uav* uav);
    void acceptUavRemoving(Uav* uav);
    void acceptRouteSelectionChanging(Route* route);
    void removeRouteIfSelected(Route* route);

    void selectRoute(Route* route);
    void startEditRoute();
    void stopEditRoute();
    void uploadEditableRouteToUav();
    void clearSelectedWaypoint();
    void selectWaypoint(std::size_t index);
    void selectWaypoints(const std::vector<std::size_t>& indexes);

    void centerOnRoute(const mccuav::Route* route);
private:
    Uav* selectedUav() const {return _selectedUav;}
    void setEditing(bool editing);

    Uav*    _selectedUav;
    Route*  _selectedRoute;

    bool    _isEditing;

    Q_DISABLE_COPY(RoutesController)
};

class MCC_UAV_DECLSPEC RoutesControllerPluginData : public mccplugin::PluginData {
public:
    static constexpr const char* id = "mcc::RoutesControllerPluginData";

    RoutesControllerPluginData(RoutesController* routesController);
    ~RoutesControllerPluginData();

    RoutesController* routesController();
    const RoutesController* routesController() const;

private:
    Rc<RoutesController> _routesController;
};
}
