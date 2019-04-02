#include "mcc/ui/TableEditWidget.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QMenu>
#include <QAction>
#include <QHeaderView>
#include <QToolButton>
#include <QApplication>

namespace mccui {

TableEditWidget::TableEditWidget(QWidget* parent)
    : QWidget(parent)
{
    _addMenu = new QMenu(this);

    _addButton = new QPushButton("Добавить");
    _addButton->setMenu(_addMenu);
    _removeButton = new QPushButton("Удалить");
    _removeButton->setEnabled(false);
    _moveUpButton = new QToolButton;
    _moveUpButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowUp));
    _moveUpButton->setEnabled(false);
    _moveDownButton = new QToolButton;
    _moveDownButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowDown));
    _moveDownButton->setEnabled(false);
    _tableView = new QTableView;

    _tableView->horizontalHeader()->setStretchLastSection(true);
    _tableView->setSelectionMode(QTableView::SingleSelection);
    _tableView->setSelectionBehavior(QTableView::SelectRows);
    _tableView->setWordWrap(false);

    auto arrowLayout = new QVBoxLayout;
    arrowLayout->addWidget(_moveUpButton);
    arrowLayout->addWidget(_moveDownButton);
    arrowLayout->addStretch();

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(_addButton);
    buttonLayout->addWidget(_removeButton);
    buttonLayout->addStretch();

    auto mainLayout = new QGridLayout;
    mainLayout->addWidget(_tableView, 0, 0);
    mainLayout->addLayout(buttonLayout, 1, 0);
    mainLayout->addLayout(arrowLayout, 0, 1);

    setLayout(mainLayout);

    connect(_removeButton, &QPushButton::clicked, this, [this]() {
        if (_model.isNone()) {
            return;
        }
        QModelIndex current = _tableView->selectionModel()->currentIndex();
        _removeButton->setEnabled(false);
        if (!current.isValid()) {
            return;
        }
        if (_model->removeRows(current.row(), 1, current.parent())) {
            updateUpDownButtons();
        }
    });

    connect(_moveUpButton, &QPushButton::clicked, this, [this]() {
        if (_model.isNone()) {
            return;
        }
        QModelIndex current = _tableView->selectionModel()->currentIndex();
        if (!current.isValid()) {
            return;
        }
        QModelIndex parent = current.parent();
        if (_model->moveRow(parent, current.row(), parent, current.row() - 1)) {
            updateUpDownButtons();
        }
    });

    connect(_moveDownButton, &QPushButton::clicked, this, [this]() {
        if (_model.isNone()) {
            return;
        }
        QModelIndex current = _tableView->selectionModel()->currentIndex();
        if (!current.isValid()) {
            return;
        }
        QModelIndex parent = current.parent();
        if (_model->moveRow(parent, current.row(), parent, current.row() + 2)) { //ebanoe kote
            updateUpDownButtons();
        }
    });

    updateUpDownButtons();
}

TableEditWidget::~TableEditWidget()
{
}

void TableEditWidget::setModel(bmcl::OptionPtr<QAbstractItemModel> model)
{
    _model = model;
    _tableView->setModel(model.data());

    // disconnect?
    connect(_tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection& currentSelection, const QItemSelection& previous) {
        _removeButton->setEnabled(currentSelection.size() >= 1);
        updateUpDownButtons();
        if (currentSelection.size() != 0) {
            emit indexSelected(currentSelection.indexes()[0]);
        }
    });

    updateUpDownButtons();
}

void TableEditWidget::addAddAction(QAction* action)
{
    _addMenu->addAction(action);
}

void TableEditWidget::updateUpDownButtons()
{
    if (_model.isNone()) {
        return;
    }
    QModelIndex current = _tableView->selectionModel()->currentIndex();

    int size = _model->rowCount(QModelIndex());
    bool upEnabled = true;
    bool downEnabled = true;
    if (size <= 1 || !current.isValid()) {
        upEnabled = false;
        downEnabled = false;
    } else {
        if (current.row() < 1) {
            upEnabled = false;
        }
        if (current.row() >= (size - 1)) {
            downEnabled = false;
        }
    }

    _moveDownButton->setEnabled(downEnabled);
    _moveUpButton->setEnabled(upEnabled);
}

void TableEditWidget::setRemoveButtonEnabled(bool flag)
{
    _removeButton->setEnabled(flag);
}
}
