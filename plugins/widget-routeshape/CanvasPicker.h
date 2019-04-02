#pragma once

#include <QObject>
#include <QPoint>
#include <qaction.h>

#include "mcc/Config.h"
#include "mcc/geo/Geod.h"
#include "mcc/ui/Fwd.h"
#include "mcc/uav/Fwd.h"

class QPoint;
class QCustomEvent;
class QwtPlot;
class QwtPlotCurve;
class RouteCurve;

class CanvasPicker : public QObject
{
    Q_OBJECT
public:
    explicit CanvasPicker(QwtPlot *plot);
    virtual bool eventFilter(QObject*, QEvent*);

    void setRoute(mccuav::Route* route);
    void setCurve(RouteCurve* curve);
signals:
    void markerSelectionChanged();

private:
    void select(const QPoint& pos, bool multipleSelect);
    int mouseOnPointIndex(const QPoint& pos);
    void move(const QPoint&, int pointIndex);
    void move(const QPoint&, const std::vector<size_t>& indexes);
    void moveBy(int dx, int dy, int pointIndex);
    void shiftPointCursor(bool up);
    void manageToolTipShowing(const QPoint& pos);

    void showEditWidget();
    void addPoint(const QPoint& pos);
    void removeSelectedPoint();
    void makeSelectedPointActive();

    QwtPlot* plot();
    const QwtPlot* plot() const;
    void applyChanges();

    RouteCurve*    _selectedCurve;
    mccuav::Route* _route;
    bool           _isMoving;
    size_t         _movingPointIndex;
    bool           _tipIsShowing;
    QPoint         _clickedPoint;
    QAction*       _addAction;
    QAction*       _removeAction;
    QAction*       _makeActiveAction;
    QAction*       _configAction;

    mccgeo::Geod   _WGS84;
};
