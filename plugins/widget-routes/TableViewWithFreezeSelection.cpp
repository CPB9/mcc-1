#include "TableViewWithFreezeSelection.h"

#include <QTableView>
#include <QDebug>

TableViewWithFreezeSelection::TableViewWithFreezeSelection(QWidget* parent)
    : QTableView(parent)
{
}

QItemSelectionModel::SelectionFlags TableViewWithFreezeSelection::selectionCommand(const QModelIndex &index, const QEvent *event) const
{
    QItemSelectionModel::SelectionFlags flags = QAbstractItemView::selectionCommand(index, event);

    auto selectedIndexes = selectionModel()->selectedIndexes();
    if (flags == QItemSelectionModel::ClearAndSelect || selectedIndexes.isEmpty())
    {
        return flags;
    }

    if (index.column() == selectedIndexes.first().column())
    {
        return flags;
    }
    else if (index.column() != selectedIndexes.first().column() && (flags & QItemSelectionModel::SelectionFlag::Toggle))
    {
        return QItemSelectionModel::SelectionFlag::NoUpdate;
    }
    return QItemSelectionModel::SelectionFlag::NoUpdate;
}
