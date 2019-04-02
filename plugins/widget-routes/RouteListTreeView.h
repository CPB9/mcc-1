#pragma once

#include "mcc/Config.h"

#include <QTreeView>

class RouteListTreeView : public QTreeView
{
    Q_OBJECT
public:
    RouteListTreeView(QWidget* parent);

    void setCanChangeSelection(bool canChange);

signals:
    void clearRouteSelection();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    bool _canChangeSelection;
};
