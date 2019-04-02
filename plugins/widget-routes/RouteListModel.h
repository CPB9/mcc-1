#pragma once

#include "mcc/Config.h"
#include "mcc/uav/Fwd.h"

#include <QAbstractTableModel>
#include <QIcon>
#include <QFont>

class RouteListModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Roles
    {
        RouteId = Qt::UserRole + 1,
        RouteVisibility,
        EnabledRole,
        HiddenRole
    };

    RouteListModel(QObject* parent = 0);

    void setDevice(mccuav::Uav* dev);
    void setEmptyDevice();

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

private slots:
    void routeAdded(mccuav::Route* route);
    void routeRemoved(mccuav::Route* route);
    void activeRouteChanged(mccuav::Route* route);
    void routeVisibilityChanged(bool visible);
    void routeEnabledChanged(bool enabled);
    void routeHiddenChanged(bool enabled);

private:
    mccuav::Uav* _device;

    QFont _boldFont;
    QIcon _activeRouteIcon;
};

