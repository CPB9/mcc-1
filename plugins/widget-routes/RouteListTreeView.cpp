#include "RouteListTreeView.h"

#include <QTreeView>
#include <QMouseEvent>

RouteListTreeView::RouteListTreeView(QWidget* parent)
    : QTreeView(parent)
    , _canChangeSelection(true)
{
}

void RouteListTreeView::setCanChangeSelection(bool canChange)
{
    _canChangeSelection = canChange;
}

void RouteListTreeView::mousePressEvent(QMouseEvent* event)
{
    if (!_canChangeSelection)
    {
        event->accept();
        return;
    }

    QModelIndex currentItem = indexAt(event->pos());

    if (!currentItem.isValid())
    {
        clearSelection();
        emit clearRouteSelection();
    }

    QTreeView::mousePressEvent(event);
}

void RouteListTreeView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        if (selectionModel()->selectedIndexes().empty())
            return;

        QModelIndex index = model()->index(selectionModel()->selectedIndexes().first().row(), 1);
        emit doubleClicked(index);
        event->accept();
    }
    return QTreeView::keyPressEvent(event);
}
