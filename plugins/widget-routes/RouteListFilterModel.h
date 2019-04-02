#pragma once

#include <QSortFilterProxyModel>

#include "mcc/Config.h"
#include "RouteListModel.h"

class RouteListFilterModel : public QSortFilterProxyModel
{
public:
    RouteListFilterModel(QObject* parent = 0);

    bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
};
