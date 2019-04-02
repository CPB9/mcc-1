#include "mcc/ide/view/ContainerWidget.h"
#include "mcc/path/Paths.h"
#include "mcc/res/Resource.h"

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QMenu>
#include <QStyle>
#include <QWidget>

#include <FloatingDockContainer.h>

#include <bmcl/Logging.h>

namespace mccide {

ContainerWidget::ContainerWidget(QWidget* parent)
    : ads::CDockManager(parent)
{
    _layoutsMenu = new QMenu("Расположение", this);
    _layoutsMenu->setIcon(QIcon(":/docking.png"));
    _widgetsMenu = new QMenu("Инструменты", this);
    _widgetsMenu->setIcon(QIcon(":/advancedsettings.png"));

//    auto createLayoutAction = _layoutsMenu->addAction("Сохранить...");
    //connect(createLayoutAction, &QAction::triggered, this, &ContainerWidget::hideAllWidgets);

    auto hideAllAction = _layoutsMenu->addAction(mccres::loadIcon(mccres::ResourceKind::CancelButtonIcon), "Закрыть все");
    connect(hideAllAction, &QAction::triggered, this, &ContainerWidget::hideAllWidgets);

    initStyleSheet();

    connect(this, &ads::CDockManager::stateRestored, this,
            []()
            {
                BMCL_DEBUG() << "STATE CHANGED";
            }
    );
    connect(_layoutsMenu, &QMenu::aboutToShow, this, &ContainerWidget::updateLayoutsMenu);
    connect(_layoutsMenu, &QMenu::triggered, this,
            [this, hideAllAction](QAction* action)
            {
                if(action == hideAllAction)
                    return;
                openPerspective(action->text());
            }
    );
}

void ContainerWidget::addWidget(QWidget* widget, ads::DockWidgetArea area, bool autoHide, QMenu* menu)
{
    auto dockWidget = new ads::CDockWidget(widget->windowTitle());
    dockWidget->setWidget(widget);
    dockWidget->setObjectName(dockWidget->windowTitle());
    dockWidget->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
    auto w = addDockWidget(area, dockWidget);
    if(autoHide)
        dockWidget->toggleView(false);
    connect(w, &ads::CDockAreaWidget::currentChanged, this, &ContainerWidget::requestSave);
    if (!menu)
        menu = _widgetsMenu;
    QAction* newAction = dockWidget->toggleViewAction();
    newAction->setData(widget->objectName());

    if(widget->windowIcon().cacheKey() != windowIcon().cacheKey())
        newAction->setIcon(widget->windowIcon());

    insertMenuAction(newAction, menu);
}

void ContainerWidget::addFloatingWidget(QWidget* widget)
{
    auto dockWidget = new ads::CDockWidget(widget->windowTitle());
    dockWidget->setWidget(widget);
    dockWidget->setObjectName(dockWidget->windowTitle());
    dockWidget->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
    auto floatingWidget = new ads::CFloatingDockContainer(this);
    auto w = floatingWidget->dockContainer()->addDockWidget(ads::CenterDockWidgetArea, dockWidget);
    floatingWidget->show();
    connect(w, &ads::CDockAreaWidget::currentChanged, this, &ContainerWidget::requestSave);
}

void ContainerWidget::removeWidget(QWidget* widget)
{
    auto dockWidget = findDockWidget(widget->objectName());
    if (!dockWidget)
    {
        BMCL_WARNING() << "ContainerWidget::removeWidget " << "не найден док-виджет для удаления!";
        return;
    }
     removeDockWidget(dockWidget);
     widget->deleteLater();
}

void ContainerWidget::addDialog(QDialog* dialog)
{
    QAction* newAction = new QAction(dialog->objectName(), dialog);
    connect(newAction, &QAction::triggered, this,
            [dialog]()
    {
        dialog->show();
        dialog->activateWindow();
    });
    connect(this, &ContainerWidget::widgetsHidden, dialog, &QDialog::hide);

    if(dialog->windowIcon().cacheKey() != windowIcon().cacheKey())
        newAction->setIcon(dialog->windowIcon());

    insertMenuAction(newAction, _widgetsMenu);
}

void ContainerWidget::initStyleSheet()
{
    QFile f(":ads/stylesheets/default-windows.css");
    if (f.open(QFile::ReadOnly))
    {
        const QByteArray ba = f.readAll();
        f.close();
        qApp->setStyleSheet(QString(ba));
    }
}

void ContainerWidget::insertMenuAction(QAction* newAction, QMenu* menu)
{
    bool inserted(false);
    for(QAction* act : _widgetsMenu->actions())
    {
        if(act->menu() != nullptr)
            continue;

        if(QString::compare(act->text(), newAction->text(), Qt::CaseInsensitive) >= 0)
        {
            menu->insertAction(act, newAction);
            inserted = true;
            break;
        }
    }
    if(!inserted)
        menu->addAction(newAction);
}

QMenu* ContainerWidget::layoutsMenu() const
{
    return _layoutsMenu;
}

QMenu* ContainerWidget::widgetsMenu() const
{
    return _widgetsMenu;
}

void ContainerWidget::hideAllWidgets()
{
    for (auto dockArea : this->openedDockAreas())
    {
        for (auto dockWidget : dockArea->openedDockWidgets())
        {
            dockWidget->toggleView(false);
        }
    }
    for (auto fw : this->floatingWidgets())
    {
        fw->hide();
    }
    emit widgetsHidden();
}

void ContainerWidget::updateLayoutsMenu()
{
//     _layoutsMenu->clear();
//     for (const auto& p : this->perspectiveNames())
//     {
//         _layoutsMenu->addAction(p);
//     }
}


void ContainerWidget::raiseWidget(const QString& objectName)
{
    auto w = this->findDockWidget(objectName);
    if (!w)
        return;
    w->toggleView(true);
    w->activateWindow();
}

void ContainerWidget::reduceWidget(const QString& objectName)
{
    auto w = this->findDockWidget(objectName);
    if (!w)
        return;
    w->toggleView(false);
}
}
