#include "RouteListModel.h"

#include <QAbstractTableModel>
#include <QApplication>
#include <QDebug>
#include <QFont>
#include <QIcon>
#include <QStyle>

#include "mcc/uav/Uav.h"
#include "mcc/uav/Route.h"

RouteListModel::RouteListModel(QObject* parent)
    : QAbstractTableModel(parent)
    , _device(nullptr)
{
    _boldFont.setBold(true);
    _activeRouteIcon = QApplication::style()->standardPixmap(QStyle::SP_ArrowRight);
}

void RouteListModel::setDevice(mccuav::Uav* dev)
{
    setEmptyDevice();

    _device = dev;

    connect(_device, &mccuav::Uav::routeAdded, this, &RouteListModel::routeAdded);
    connect(_device, &mccuav::Uav::routeRemoved, this, &RouteListModel::routeRemoved);
    connect(_device, &mccuav::Uav::activeRouteChanged, this, &RouteListModel::activeRouteChanged);

    for (auto route : _device->routes())
    {
        connect(route, &mccuav::Route::userVisibilityFlagChanged,     this, &RouteListModel::routeVisibilityChanged);
        connect(route, &mccuav::Route::readOnlyFlagChanged,       this, &RouteListModel::routeVisibilityChanged);
        connect(route, &mccuav::Route::enabledFlagChanged,        this, &RouteListModel::routeEnabledChanged);
        connect(route, &mccuav::Route::hiddenFlagChanged, this, &RouteListModel::routeHiddenChanged);
    }

    beginResetModel();
    endResetModel();
}

void RouteListModel::setEmptyDevice()
{
    if (_device != nullptr)
    {
        disconnect(_device, &mccuav::Uav::routeAdded, this, &RouteListModel::routeAdded);
        disconnect(_device, &mccuav::Uav::routeRemoved, this, &RouteListModel::routeRemoved);
        disconnect(_device, &mccuav::Uav::activeRouteChanged, this, &RouteListModel::activeRouteChanged);

        for (auto route : _device->routes())
        {
            disconnect(route, &mccuav::Route::userVisibilityFlagChanged, this, &RouteListModel::routeVisibilityChanged);
            disconnect(route, &mccuav::Route::readOnlyFlagChanged, this, &RouteListModel::routeVisibilityChanged);
            disconnect(route, &mccuav::Route::enabledFlagChanged, this, &RouteListModel::routeEnabledChanged);
            disconnect(route, &mccuav::Route::hiddenFlagChanged, this, &RouteListModel::routeHiddenChanged);
        }
    }

    _device = nullptr;
    beginResetModel();
    endResetModel();
}

QVariant RouteListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Vertical)
        return QVariant();

    switch (section)
    {
    case 0:
        return "";
    case 1:
        return "Идентификатор";
    case 2:
        return "Имя";
    }

    return QVariant();
}

Qt::ItemFlags RouteListModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractTableModel::flags(index);
    if (!index.data(EnabledRole).toBool())
    {
        flags &= ~Qt::ItemIsEnabled;
        flags &= ~Qt::ItemIsEditable;
        flags &= ~Qt::ItemIsSelectable;
    }

    if (index.column() == 2)
        flags |= Qt::ItemIsUserCheckable;

    return flags;
}

int RouteListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    if (_device == nullptr)
        return 0;

    return _device->routes().count();
}

int RouteListModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 3; //name, id, color
}

QVariant RouteListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    const auto& routes = _device->routes();

    int routeIndex = index.row();

    if (routeIndex >= routes.count())
    {
        Q_ASSERT(false);
        return QVariant();
    }

    auto route = routes[routeIndex];

    bool isActiveRoute = (_device->activeRoute() == route);
    bool isSelectedRoute = (_device->selectedRoute() != nullptr && route->id() == _device->selectedRoute()->id());

    if (role == RouteId)
    {
        return route->id();
    }
    else if (role == RouteVisibility)
    {
        return route->isUserVisible();
    }
    else if (role == EnabledRole)
    {
        return route->isEnabled();
    }
    else if (role == HiddenRole)
    {
        return route->isHidden();
    }
    else if (role == Qt::CheckStateRole && index.column() == 2)
    {
        if(route->isUserVisible()) return Qt::Checked;
        else return Qt::Unchecked;
    }

    if (role == Qt::BackgroundRole && isSelectedRoute && index.column() != 0)
    {
        return QColor("#1d3dec");
    }

    if (role == Qt::ForegroundRole && isSelectedRoute && index.column() != 0)
    {
        return QColor(Qt::white);
    }

    if (index.column() == 2)
    {
        if (role == Qt::DisplayRole)
            return route->name();
        else if (role == Qt::DecorationRole && isActiveRoute)
            return _activeRouteIcon;
        else if (role == Qt::FontRole && isActiveRoute)
            return _boldFont;
        else
            return QVariant();
    }
    else if (index.column() == 1)
    {
        if (role == Qt::DisplayRole)
            return route->id();
        else if (role == Qt::FontRole && isActiveRoute)
            return _boldFont;
        return QVariant();
    }
    else if (index.column() == 0)
    {
        if (role == Qt::BackgroundRole)
            return route->pen().color();
        return QVariant();
    }

    return QVariant();
}


QModelIndex RouteListModel::parent(const QModelIndex& child) const
{
    BMCL_UNUSED(child);
    return QModelIndex();
}

bool RouteListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    qDebug() << value;
    if (role == Qt::CheckStateRole && index.column() == 2)
    {
        const auto& routes = _device->routes();
        int routeIndex = index.row();

        if (routeIndex >= routes.count())
        {
            Q_ASSERT(false);
            return false;
        }

        auto route = routes[routeIndex];
        bool isVisible = data(index, RouteVisibility).toBool();

        route->setUserVisibility(!isVisible);
        return true;
    }

    return QAbstractTableModel::setData(index, value, role);
}

void RouteListModel::routeAdded(mccuav::Route* route)
{
    connect(route, &mccuav::Route::userVisibilityFlagChanged, this, &RouteListModel::routeVisibilityChanged);
    connect(route, &mccuav::Route::readOnlyFlagChanged, this, &RouteListModel::routeVisibilityChanged);
    connect(route, &mccuav::Route::enabledFlagChanged, this, &RouteListModel::routeEnabledChanged);
    connect(route, &mccuav::Route::hiddenFlagChanged, this, &RouteListModel::routeHiddenChanged);
    beginResetModel();
    endResetModel();
}

void RouteListModel::routeRemoved(mccuav::Route* route)
{
    disconnect(route, &mccuav::Route::userVisibilityFlagChanged, this, &RouteListModel::routeVisibilityChanged);
    disconnect(route, &mccuav::Route::readOnlyFlagChanged, this, &RouteListModel::routeVisibilityChanged);
    disconnect(route, &mccuav::Route::enabledFlagChanged, this, &RouteListModel::routeEnabledChanged);
    disconnect(route, &mccuav::Route::hiddenFlagChanged, this, &RouteListModel::routeHiddenChanged);

    beginResetModel();
    endResetModel();
}

void RouteListModel::activeRouteChanged(mccuav::Route* route)
{
    Q_UNUSED(route);
    beginResetModel();
    endResetModel();
}

void RouteListModel::routeVisibilityChanged(bool visible)
{
    Q_UNUSED(visible);
    beginResetModel();
    endResetModel();
}

void RouteListModel::routeEnabledChanged(bool enabled)
{
    Q_UNUSED(enabled);
    beginResetModel();
    endResetModel();
}

void RouteListModel::routeHiddenChanged(bool enabled)
{
    Q_UNUSED(enabled);
    beginResetModel();
    endResetModel();
}
