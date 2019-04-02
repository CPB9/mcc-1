#pragma once

#include <DockManager.h>
#include <DockWidget.h>
#include <DockAreaWidget.h>

#include <QByteArray>
#include <QMap>

class QMenu;

namespace mccide {

class ContainerWidget : public ads::CDockManager
{
    Q_OBJECT
public:
    ContainerWidget(QWidget* parent = nullptr);
    void addWidget(QWidget* widget, ads::DockWidgetArea area = ads::CenterDockWidgetArea, bool autoHide = true, QMenu* menu = nullptr);
    void addFloatingWidget(QWidget* widdget);
    void removeWidget(QWidget* widget);
    void addDialog(QDialog* dialog);

    QMenu* layoutsMenu() const;
    QMenu* widgetsMenu() const;

public slots:
    void hideAllWidgets();
    void updateLayoutsMenu();
    void raiseWidget(const QString& objectName);
    void reduceWidget(const QString& objectName);

signals:
    void requestSave();
    void widgetsHidden();

private:
    void initStyleSheet();
    void insertMenuAction(QAction* newAction, QMenu* menu);

private:
    QMenu* _layoutsMenu;
    QMenu* _widgetsMenu;
};
}
