#pragma once

#include "mcc/Config.h"

#include <bmcl/OptionPtr.h>

#include <QWidget>

class QPushButton;
class QTableView;
class QToolButton;
class QAbstractItemModel;
class QMenu;
class QAction;
class QModelIndex;

namespace mccui {

class MCC_UI_DECLSPEC TableEditWidget : public QWidget {
    Q_OBJECT
public:
    TableEditWidget(QWidget* parent = nullptr);
    ~TableEditWidget();

    void setModel(bmcl::OptionPtr<QAbstractItemModel> model);
    void addAddAction(QAction* action);

    void setRemoveButtonEnabled(bool flag);

signals:
    void indexSelected(const QModelIndex& idx);

private:
    void updateUpDownButtons();

    QMenu* _addMenu;
    QPushButton* _addButton;
    QPushButton* _removeButton;
    QToolButton* _moveUpButton;
    QToolButton* _moveDownButton;
    QTableView* _tableView;
    bmcl::OptionPtr<QAbstractItemModel> _model;
};
}
