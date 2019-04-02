#include "mcc/ui/TreeViewWithKeyboard.h"

#include <QTreeView>
#include <QKeyEvent>

namespace mccui {

TreeViewWithKeyboard::TreeViewWithKeyboard(QWidget* parent)
    : QTreeView(parent)
{

}

void TreeViewWithKeyboard::keyPressEvent(QKeyEvent* event)
{
    QModelIndex currentIndex = model()->index(this->currentIndex().row(), 0, this->currentIndex().parent());

    if (event->key() != Qt::Key_Return && event->key() != Qt::Key_Enter)
    {
        QTreeView::keyPressEvent(event);
        return;
    }

    if (model()->rowCount(currentIndex) == 0)
        emit doubleClicked(currentIndex);
    else if (isExpanded(currentIndex))
        collapse(currentIndex);
    else
        expand(currentIndex);

}
}
