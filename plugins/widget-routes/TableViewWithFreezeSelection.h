#pragma once

#include "mcc/Config.h"

#include <QTableView>

class TableViewWithFreezeSelection : public QTableView
{
public:
    explicit TableViewWithFreezeSelection(QWidget* parent = 0);

    QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index, const QEvent *event = 0) const override;
};
