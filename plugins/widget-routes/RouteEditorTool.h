#pragma once

#include "mcc/Config.h"
#include "mcc/map/Fwd.h"
#include "mcc/map/MapRect.h"
#include "mcc/ui/Settings.h"
#include "mcc/ui/Fwd.h"
#include "mcc/uav/Fwd.h"
#include "mcc/msg/Fwd.h"
#include "mcc/ui/Rc.h"
#include <QWidget>
#include <QItemSelection>
#include <functional>

class RouteListModel;
class RouteListFilterModel;
class RouteListTreeView;
class MissionPlanModel;
class ScanAreaDialog;

class QMenu;
class QFrame;
class QPushButton;
class QCheckBox;
class QLabel;
class QTableView;

class RoutesListWidget : public QWidget
{
    Q_OBJECT
public:
    RoutesListWidget(QWidget* parent, mccui::Settings* settings, mccuav::UavController* uavController, mccuav::RoutesController* routesController, mccmsg::ProtocolController* pController);
    void setEmptyUav();
    void setRoute(mccuav::Route* route, mccuav::Uav* uav);
private:
    void updateButtons(mccuav::Route* route);
    bmcl::OptionPtr<mccuav::Route> currentRoute() const;
private slots:
    void selectRoute(const QItemSelection &selected, const QItemSelection &deselected);
    void routeDoubleClicked(const QModelIndex& index);
    void contextMenuRequested(const QPoint& p);

    void copyRouteButtonClicked();
    void clearRouteButtonClicked();
    void uploadRouteButtonClicked();
    void saveRouteButtonClicked();
    void loadRouteButtonClicked();

    void setRouteActive(mccuav::Route* route);
    void setRouteEmpty();
    void changeRouteDirection(mccuav::Route* route);
    void loadFromDisk(mccuav::Route* route);
    void saveToDisk(mccuav::Route* route);
    void exportToKml(mccuav::Route* route);
    void exportAllToKml();
    void saveAllToDisk();
private:
    mccui::Rc<mccui::SettingsWriter> _routesDirWriter;
    RouteListModel* _model;
    RouteListFilterModel* _filterModel;
    mccuav::UavController* _uavController;
    mccuav::RoutesController* _routesController;
    mccmsg::ProtocolController* _pController;
    RouteListTreeView* _routesView;
    QPushButton* _editRouteButton;
    QPushButton* _copyRouteButton;
    QPushButton* _clearRouteButton;
    QPushButton* _uploadRouteButton;
    QPushButton* _saveRouteButton;
    QPushButton* _loadRouteButton;
};

class EditedRouteHeaderWidget : public QWidget
{
public:
    EditedRouteHeaderWidget(QWidget* parent, mccuav::RoutesController* routesController);

    void setRoute(mccuav::Route* route);
private:
    QLabel* _color;
    QLabel* _info;
};

class RoutePropertiesWidget : public QWidget
{
    Q_OBJECT
public:
    RoutePropertiesWidget(QWidget* parent, mccuav::RoutesController* routesController);
    void setRoute(mccuav::Route* r);
private slots:
    void updateButtons();
private:
    mccuav::Route* _route;
    mccuav::RoutesController* _routesController;
    QCheckBox* _loopCheckbox;
    QCheckBox* _revesedCheckbox;
    QCheckBox* _showPointsOnlyCheckbox;
};

class WaypointsEditorWidget : public QWidget
{
    Q_OBJECT
public:
    WaypointsEditorWidget(QWidget* parent, const mccmap::MapRect* rect, const mccui::CoordinateSystemController* csController, mccui::Settings* settings, mccuav::UavController* uavController, mccuav::RoutesController* routesController);

    void setRoute(mccuav::Route* route);
private slots:
    void contextMenuRequested(const QPoint& p);

    void waypointDoubleClicked(const QModelIndex& index);
    void addButtonPressed();
    void moveUpButtonPressed();
    void moveDownButtonPressed();
    void removeButtonPressed();
    void inverseButtonPressed();

    void scanAreaPressed();
    void scanRectPressed();
    void scanAccepted();
    void scanRejected();

    void clearActivePoint();
    void applyToSelected();
    void applyToAll();
private:
    void updateButtons();
    mccui::Rc<const mccmap::MapRect>    _mapRect;
    mccuav::Route* _route;
    mccuav::UavController* _uavController;
    mccuav::RoutesController* _routesController;
    MissionPlanModel* _model;

    QTableView*     _waypointsTable;
    QFrame*         _editButtons;
    QPushButton*    _addButton;
    QPushButton*    _moveUpButton;
    QPushButton*    _moveDownButton;
    QPushButton*    _removeButton;
    QPushButton*    _inverseButton;

    ScanAreaDialog* _scanDialog;
    QPushButton*    _scanRectButton;
    QPushButton*    _scanAreaButton;
};

class RouteEditorTool : public QWidget
{
    Q_OBJECT
public:
    RouteEditorTool(const mccui::CoordinateSystemController* csController,
                    const mccmap::MapRect* rect,
                    mccui::Settings* settings,
                    mccuav::RoutesController* routesController,
                    mccuav::UavController* uavController,
                    mccmsg::ProtocolController* pController,
                    QWidget* parent = nullptr);
    ~RouteEditorTool();

private:
    QString  _filenameFilter;
    mccui::Rc<const mccmap::MapRect>    _mapRect;
    mccui::Rc<mccui::SettingsWriter>    _routesDirWriter;
    mccui::Rc<mccuav::RoutesController> _routesController;
    mccui::Rc<mccuav::UavController>    _uavController;

    RoutesListWidget*           _routesListWidget;
    EditedRouteHeaderWidget*    _editedRouteHeaderWidget;
    RoutePropertiesWidget*      _routePropertiesWidget;
    WaypointsEditorWidget*      _waypointsEditorWidget;
};
