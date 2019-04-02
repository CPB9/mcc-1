#pragma once

#include "mcc/Config.h"
#include "mcc/uav/Fwd.h"
#include "mcc/uav/Rc.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Rc.h"
#include "mcc/uav/Route.h"

#include <QAbstractTableModel>

class MissionPlanModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum class Columns
    {
        Index,
        Latitude,
        Longitude,
        Altitude,
        Speed,
        WaypointType,
        Count
    };

    MissionPlanModel(const mccui::CoordinateSystemController* csController);
    ~MissionPlanModel();

    void setRoute(mccuav::Route* route);
    void setEmptyRoute();
    void setActivePoint(int point);

    int           rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int           columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant      data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant      headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool          setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    static int roleForColumn(Columns column);
    static mccuav::Route::EditMode columnToEditMode(Columns column);
    void setEnabled(bool mode);
    bool isEnabled() const;
public slots:
    void moveWaypointUp(int index);
    void moveWaypointDown(int index);
    void removeWaypoint(int index);

private:
    mccuav::Route* _route;
    bool _enabled;
    int _activePoint;
    mccuav::Rc<const mccui::CoordinateSystemController> _csController;
};
