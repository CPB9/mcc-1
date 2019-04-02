#include "mcc/uav/RoutesController.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/Route.h"
#include "mcc/uav/Uav.h"

#include "mcc/geo/Bbox.h"

#include <bmcl/Logging.h>

#include <QMessageBox>

namespace mccuav {

RoutesController::~RoutesController()
{
}

RoutesController::RoutesController(QObject *parent)
    : QObjectRefCountable<QObject>(parent)
    , _selectedUav(nullptr)
    , _selectedRoute(nullptr)
    , _isEditing(false)
{
}

size_t RoutesController::availableRoutesCount() const
{
    if(selectedUav())
        return selectedUav()->routes().size();

    return 0;
}

Route*RoutesController::editableRoute() const
{
    if(isEditing() && selectedRoute() != nullptr)
    {
        return selectedRoute()->buffer();
    }

    return nullptr;
}

bmcl::Option<const std::vector<std::size_t>&> RoutesController::selectedWaypoint() const
{
    if(selectedRoute() != nullptr)
    {
        if(isEditing())
        {
            mccuav::Route* editable = editableRoute();
            if(editable != nullptr)
                return editable->selectedPointIndexes();
        }
        else
        {
            return selectedRoute()->selectedPointIndexes();
        }
    }

    return bmcl::None;
}

void RoutesController::acceptUavSelection(Uav* uav)
{
    if(uav == selectedUav())
        return;

    stopEditRoute();

    if(selectedUav() != nullptr)
        selectedUav()->disconnect(this);

    _selectedUav = uav;

    if (_selectedUav)
    {
        connect(selectedUav(), &Uav::routeSelectionChanged, this, &RoutesController::acceptRouteSelectionChanging);
        connect(selectedUav(), &Uav::routeUploaded, this, &RoutesController::stopEditRoute);
        connect(selectedUav(), &Uav::routeRemoved, this, &RoutesController::removeRouteIfSelected);
    }

    acceptRouteSelectionChanging(selectedUav() != nullptr ? selectedUav()->selectedRoute() : nullptr);
}

void RoutesController::acceptUavRemoving(Uav* uav)
{
    if(uav == selectedUav())
    {
        acceptUavSelection(nullptr);
    }
}

void RoutesController::acceptRouteSelectionChanging(Route* route)
{
    if(route == selectedRoute())
        return;

    // Previous route
    if(selectedRoute() != nullptr)
    {
        selectedRoute()->setState(RouteState::Inactive);

        selectedRoute()->disconnect(this);

        if(selectedRoute()->buffer() != nullptr)
            selectedRoute()->buffer()->disconnect(this);
    }

    _selectedRoute = route;

    emit selectedRouteChanged(selectedRoute(), selectedUav());

    if(selectedRoute() != nullptr)
    {
        selectedRoute()->setState(RouteState::Selected);

        connect(selectedRoute(), &mccuav::Route::selectedWaypointsChanged, this,
                [this]()
        {
            if(!isEditing())
                emit selectedWaypointChanged(selectedRoute()->selectedPointIndexes());
        });

        connect(selectedRoute(), &mccuav::Route::activeWaypointChanged, this,
                [this]()
        {
            if(!isEditing())
                emit activeWaypointChanged(selectedRoute()->activePointIndex());
        });

        if(selectedRoute()->buffer() != nullptr)
        {
            connect(selectedRoute()->buffer(), &mccuav::Route::selectedWaypointsChanged, this,
                    [this]()
            {
                if(isEditing())
                    emit selectedWaypointChanged(selectedRoute()->buffer()->selectedPointIndexes());
            });
            connect(selectedRoute()->buffer(), &mccuav::Route::activeWaypointChanged, this,
                    [this]()
            {
                if(isEditing())
                    emit activeWaypointChanged(selectedRoute()->buffer()->activePointIndex());
            });
        }
    }
}

void RoutesController::removeRouteIfSelected(Route* route)
{
    if(route == selectedRoute())
        selectRoute(nullptr);
}

void RoutesController::selectRoute(Route* route)
{
    if(selectedUav() == nullptr || selectedRoute() == route)
        return;

    selectedUav()->selectRoute(route);
}

void RoutesController::startEditRoute()
{
    setEditing(true);
}

void RoutesController::stopEditRoute()
{
    setEditing(false);
}

void RoutesController::uploadEditableRouteToUav()
{
    if(selectedUav() == nullptr ||
       editableRoute() == nullptr)
        return;

    std::vector<size_t> numbers;

    const mccmsg::Waypoints& wps = editableRoute()->waypointsList();
    for(int i = 0; i < wps.size(); ++i)
    {
//         if(wps[i].formation.isDefault() && wps[i].properties.testFlag(mccuav::WaypointType::Formation))
//             numbers.push_back(i + 1);
    }
    if(!numbers.empty())
    {
        QString points = QString::number(numbers.front());
        for(size_t i = 1; i < numbers.size(); ++i)
        {
            points = points + QString(", %1").arg(numbers[i]);
        }

        QString message;
        if(numbers.size() > 1)
            message = QString("Точки маршрута %1 содержат");
        else
            message = QString("Точка маршрута %1 содержит");

        if(QMessageBox::question(nullptr, "Незаданная формация",
                                 message.arg(points) + QString(" формацию по умолчанию.\nПродолжить?"))
           == QMessageBox::Yes)
        {
            selectedUav()->uploadEditableRoute();
        }
    }
    else
    {
        selectedUav()->uploadEditableRoute();
    }
}

void RoutesController::clearSelectedWaypoint()
{
    selectWaypoints({});
}

void RoutesController::selectWaypoint(std::size_t index)
{
    selectWaypoints({ index });
}

void RoutesController::selectWaypoints(const std::vector<std::size_t>& indexes)
{
    if (selectedRoute() == nullptr)
        return;

    if (isEditing())
    {
        mccuav::Route* editable = editableRoute();
        if (editable != nullptr)
            editable->setSelectedPoints(indexes);
    }
    else
        selectedRoute()->setSelectedPoints(indexes);
}

void RoutesController::centerOnRoute(const Route* route)
{
    if(route == nullptr || route->waypointsCount() == 0)
        return;

    if(isEditing() && route->buffer() != nullptr)
        route = route->buffer();

    if(route->waypointsCount() == 1)
    {
        emit routeCentering(route->waypointAt(0).position.latLon());
    }
    else
    {
        emit routeCentering(route->computeBoundindBox());
    }
}

void RoutesController::setEditing(bool editing)
{
    if(editing == isEditing())
        return;

    if(selectedRoute() != nullptr || !editing)
    {
        _isEditing = editing;

        if(isEditing() && selectedUav() != nullptr)
        {
            selectedRoute()->setState(RouteState::UnderEdit);
            selectedUav()->resetEditableRoute(selectedRoute()->id());
        }
        else
        {
            selectedRoute()->setState(RouteState::Selected);
        }

        emit routeEditingChanged(isEditing());
    }

    // Last point selecting
    mccuav::Route* editable = editableRoute();
    if(editable != nullptr && editable->waypointsCount() > 0)
    {
        editable->setSelectedPoint(editable->waypointsCount() - 1);
    }
}

RoutesControllerPluginData::RoutesControllerPluginData(RoutesController* routesController)
    : mccplugin::PluginData(id)
    , _routesController(routesController)
{
}

RoutesControllerPluginData::~RoutesControllerPluginData()
{
}

RoutesController* RoutesControllerPluginData::routesController()
{
    return _routesController.get();
}

const RoutesController* RoutesControllerPluginData::routesController() const
{
    return _routesController.get();
}
}
