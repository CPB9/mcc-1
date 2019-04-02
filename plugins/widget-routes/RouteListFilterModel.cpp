#include "RouteListFilterModel.h"
#include "RouteListModel.h"

#include <QSortFilterProxyModel>

RouteListFilterModel::RouteListFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{

}

bool RouteListFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    return !sourceModel()->data(index, RouteListModel::HiddenRole).toBool();
}

