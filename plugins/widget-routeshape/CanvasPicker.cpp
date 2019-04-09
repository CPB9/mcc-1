#include "CanvasPicker.h"
#include "RouteCurve.h"

#include "mcc/uav/Structs.h"
#include "mcc/uav/Route.h"
#include "mcc/geo/Constants.h"

#include <bmcl/Logging.h>

#include <cfloat>

#include <QApplication>
#include <qevent.h>
#include <qwhatsthis.h>
#include <qpainter.h>
#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_scale_map.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <QMenu>
#include <QContextMenuEvent>
#include <QToolTip>

CanvasPicker::CanvasPicker(QwtPlot *plot):
    QObject(plot),
    _selectedCurve(nullptr),
    _route(nullptr),
    _tipIsShowing(false),
    _WGS84(mccgeo::wgs84a<double>(), mccgeo::wgs84f<double>())
{
    _isMoving = false;

    QwtPlotCanvas *canvas = qobject_cast<QwtPlotCanvas *>(plot->canvas());
    canvas->installEventFilter(this);
    canvas->setFocusPolicy(Qt::StrongFocus);
    canvas->setFocusIndicator(QwtPlotCanvas::ItemFocusIndicator);
    canvas->setFocus();
    canvas->setMouseTracking(true);

    _addAction = new QAction(QString("Добавить точку"), this);
    connect(_addAction, &QAction::triggered, this, [this]() {
        addPoint(_clickedPoint);
    });
    _removeAction = new QAction(QString("Удалить точку"), this);
    connect(_removeAction, &QAction::triggered, this, [this]() {
        removeSelectedPoint();
    });
    _makeActiveAction = new QAction(QString("Сделать активной"), this);
    connect(_makeActiveAction, &QAction::triggered, this, [this]() {
        makeSelectedPointActive();
    });
    _configAction= new QAction(QString("Свойства точки"), this);
    connect(_configAction, &QAction::triggered, this, [this]() {
        showEditWidget();
    });
}

void CanvasPicker::addPoint(const QPoint& p){
    if (!_route || !_selectedCurve || _selectedCurve->dataSize() < 2)
        return;

    double x = plot()->invTransform(plot()->xBottom, p.x());
    double y = plot()->invTransform(plot()->yLeft, p.y());
    int index = 0;
    for (int i = 0; i < _selectedCurve->dataSize(); i++) {
        if (x > _selectedCurve->data()->sample(i).x())
            index++;
    }
    if (index == 0 || index >= _selectedCurve->data()->size())
        return;

    int index1 = index - 1;
    size_t index2 = index;
    if (_route->waypointsCount() <= index2)
        index2 = 0;
    const mccmsg::Waypoint& wp1 = _route->waypointAt(index1);
    const mccmsg::Waypoint& wp2 = _route->waypointAt(index2);

    double d = 0;
    double a = 0;
    _WGS84.inverse(wp1.position, wp2.position, &d, &a, 0);
    if (!std::isnormal(d)) {
        return;
    }
    double dx = 1000 * (x - _selectedCurve->data()->sample(index1).x());
    mccgeo::LatLon latLon2;
    _WGS84.direct(wp1.position, a, dx, &latLon2, 0);

    mccmsg::Waypoint wp(mccgeo::Position(latLon2, y), wp1.speed, 0.0);
    _selectedCurve->setSelectedIndices({ index2 });
    _route->insertWaypoint(wp, index);
}

void CanvasPicker::removeSelectedPoint()
{
    for (auto i : _selectedCurve->selectedIndices())
        _route->removeWaypoint(i);
    _selectedCurve->clearSelection();
}

void CanvasPicker::makeSelectedPointActive()
{
    if (_selectedCurve->selectedIndices().empty())
        return;
    size_t index = _selectedCurve->selectedIndices().front();
    _route->setActivePoint(index);
}

void CanvasPicker::showEditWidget() {
//     if (_selectedPoint < 0)
//         return;
//    _settings->set(_route, selecedWaypointIndex());
//    _settings->setWindowTitle("Свойства точки маршрута");
//    _settings->show();
}

QwtPlot *CanvasPicker::plot()
{
    return qobject_cast<QwtPlot *>(parent());
}

const QwtPlot *CanvasPicker::plot() const
{
    return qobject_cast<const QwtPlot *>(parent());
}

void CanvasPicker::applyChanges()
{
    if (!_route)
        return;

    if (_selectedCurve->selectedIndices().empty())
        return;

    _route->blockSignals(true);
    for (auto i : _selectedCurve->selectedIndices())
    {
        int pointIndex = i;
        if (_route->isLoop() && pointIndex == _route->waypointsCount())
        {
            pointIndex = 0;
        }
        double y = (double)_selectedCurve->sample(pointIndex).y();
        _route->setWaipointAlt(y, pointIndex);
    }
    _route->blockSignals(false);
    for (auto i : _selectedCurve->selectedIndices())
    {
        if (_route->isLoop() && i == _route->waypointsCount())
            continue;
        emit _route->waypointOnlyAltChanged(_route->waypointAt(i), i);
    }
}

void CanvasPicker::manageToolTipShowing(const QPoint& pos)
{
    QPoint pos1(pos.x() + 1, pos.y() + 25);
    int idx = mouseOnPointIndex(pos);
    if (idx >= 0) {
        if (_tipIsShowing)
            return;
        _tipIsShowing = true;
        if (idx >= _route->waypointsCount())
            idx = 0;
        const auto wp = _route->waypointAt(idx);
        QPoint gP = plot()->mapToGlobal(pos);

        QString dtext = QString("%1м (%2°, %3°)").arg(QString::number(wp.position.altitude(), 'f', 2)).arg(wp.position.latLon().latitude()).arg(wp.position.latLon().longitude());
        QToolTip::showText(gP, dtext);
    }
    else {
        _tipIsShowing = false;
        QToolTip::hideText();
    }
}

bool CanvasPicker::eventFilter(QObject *object, QEvent *event)
{
    if (plot() == NULL || object != plot()->canvas() || !_route)
        return false;

    switch (event->type())
    {
    case QEvent::Paint:
    {
        QApplication::postEvent(this, new QEvent(QEvent::User));
        break;
    }
    case QEvent::MouseButtonPress:
    {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        select(mouseEvent->pos(), mouseEvent->modifiers() == Qt::ControlModifier);
        //left click
        if (mouseEvent->button() == Qt::LeftButton)
        {
            emit markerSelectionChanged();
            _isMoving = !_selectedCurve->selectedIndices().empty();
            return true;
        }
        //right click
        else if (mouseEvent->button() == Qt::RightButton) {
            _clickedPoint = mouseEvent->pos();
            QMenu menu;
            menu.setTitle("Маршрут");
            menu.addAction(_addAction);
            menu.addAction(_removeAction);
            menu.addAction(_makeActiveAction);
            //menu.addAction(_configAction);

            bool isAnySelected = !_selectedCurve->selectedIndices().empty();
            bool isOneSelected = _selectedCurve->selectedIndices().size() == 1;
            _removeAction->setEnabled(isAnySelected);
            _makeActiveAction->setEnabled(isOneSelected);
            _configAction->setEnabled(isAnySelected);
            QPoint gP = plot()->mapToGlobal(mouseEvent->pos());
            menu.exec(gP);
        }
        return false;
    }
    case QEvent::MouseMove:
    {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (_isMoving)
        {
            move(mouseEvent->pos(), _selectedCurve->selectedIndices());
            return true;
        }
        else {
            manageToolTipShowing(mouseEvent->pos());
        }
    }
    case QEvent::MouseButtonRelease:
    {
        if (_isMoving)
        {
            _isMoving = false;
            applyChanges();
            return true;
        }
        return false;
    }
    case QEvent::KeyPress:
    {
        const QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

//        const int delta = 5;
        switch (keyEvent->key())
        {
        case Qt::Key_Right:
        {
            shiftPointCursor(true);
            return true;
        }
        case Qt::Key_Left:
        {
            shiftPointCursor(false);
            return true;
        }

        case Qt::Key_Down:
        {
//             if (_selectedPoint == -1)
//                 break;
//             moveBy(0, delta, _selectedPoint);
//             if (_route->isClosedPath())
//             {
//                 if (_selectedPoint == 0)
//                     moveBy(0, delta, _route->waypointsCount());
//                 else if (_selectedPoint == _route->waypointsCount())
//                     moveBy(0, delta, 0);
//             }
//             applyChanges();
            break;
        }
        case Qt::Key_Up:
        {
//             if (_selectedPoint == -1)
//                 break;
//             moveBy(0, -delta, _selectedPoint);
//             if (_route->isClosedPath())
//             {
//                 if (_selectedPoint == 0)
//                     moveBy(0, delta, _route->waypointsCount());
//                 else if (_selectedPoint == _route->waypointsCount())
//                     moveBy(0, delta, 0);
//             }
//             applyChanges();
            break;
        }

        case Qt::Key_Delete:
        {
            removeSelectedPoint();
        }

        default:
            break;
        }
    }
    case QEvent::MouseButtonDblClick: {
        const QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::LeftButton)
            break;
        QPoint p = mouseEvent->pos();
        addPoint(p);
        break;
    }
    default:
        break;
    }

    return QObject::eventFilter(object, event);
}

void CanvasPicker::setRoute( mccuav::Route* route)
{
    _route = route;
}

void CanvasPicker::setCurve(RouteCurve* curve)
{
    _selectedCurve = curve;
}

// Select the point at a position. If there is no point
// deselect the selected point

void CanvasPicker::select(const QPoint &pos, bool multipleSelect)
{
    double distance = 0.0;
    int idx = _selectedCurve->closestPoint(pos, &distance);
    if (distance >= 10)
    {
        _route->clearSelection();
        return;
    }

    _movingPointIndex = (size_t)idx;

    if (_route->isLoop() && idx >= _route->waypointsCount())
        idx = 0;

    if (multipleSelect)
    {
        if (_route->isPointSelected(idx))
            _route->deselectPoint(idx);
        else
            _route->selectPoint(idx);
    }
    else if (_route->isPointSelected(idx))
        return;
    else
    {
        _route->clearSelection();
        _route->selectPoint(idx);
    }
}

int CanvasPicker::mouseOnPointIndex(const QPoint& pos) {

    double distance = 0.0;

    int idx = _selectedCurve->closestPoint(pos, &distance);

    if (distance < 10)
    {
        return idx;
    }
    return -1;
}

// Move the selected point
void CanvasPicker::moveBy(int dx, int dy, int pointIndex)
{
    if (dx == 0 && dy == 0)
        return;

    if (!_selectedCurve)
        return;

//     const QPointF sample = _selectedCurve->sample(_selectedPoint);
//     const double x = plot()->transform(_selectedCurve->xAxis(), sample.x());
//     const double y = plot()->transform(_selectedCurve->yAxis(), sample.y());
//
//     move(QPoint(qRound(x + dx), qRound(y + dy)), pointIndex);
}

// Move the selected point
void CanvasPicker::move(const QPoint &pos, int pointIndex)
{
//     if (!_selectedCurve)
//         return;
//
//     QVector<double> xData((int)_selectedCurve->dataSize());
//     QVector<double> yData((int)_selectedCurve->dataSize());
//
//
//     for (size_t i = 0; i < _selectedCurve->dataSize(); i++)
//     {
//         const QPointF sample = _selectedCurve->sample(i);
//         if (i == pointIndex)
//         {
//             xData[i] = _selectedCurve->sample(i).x();
//             yData[i] = _selectedCurve->sample(i).y() + dy;
//         }
//         else
//         {
//
//             xData[i] = sample.x();
//             yData[i] = sample.y();
//         }
//     }
//     _selectedCurve->setSamples(xData, yData);
}

void CanvasPicker::move(const QPoint& pos, const std::vector<size_t>& indexes)
{
    std::vector<double> xData(_selectedCurve->dataSize());
    std::vector<double> yData(_selectedCurve->dataSize());
    for (size_t i = 0; i < _selectedCurve->dataSize(); ++i)
    {
        QPointF sample = _selectedCurve->sample(i);
        xData[i] = sample.x();
        yData[i] = sample.y();
    }

    double newY = plot()->invTransform(_selectedCurve->yAxis(), pos.y());
    double dy = newY - _selectedCurve->sample(_movingPointIndex).y();

     for (auto i : indexes)
     {
         yData[i] += dy;
     }
    _selectedCurve->setSamples(xData.data(), yData.data(), _selectedCurve->dataSize());
    /*
Enable QwtPlotCanvas::ImmediatePaint, so that the canvas has been
updated before we paint the cursor on it.
*/
    QwtPlotCanvas *plotCanvas =
        qobject_cast<QwtPlotCanvas *>(plot()->canvas());

    plotCanvas->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, true);
    plot()->replot();
    plotCanvas->setPaintAttribute(QwtPlotCanvas::ImmediatePaint, false);
}

void CanvasPicker::shiftPointCursor(bool up)
{
    if (!_selectedCurve)
        return;

//     int index = _selectedPoint + (up ? 1 : -1);
//     index = (index + (int)_selectedCurve->dataSize()) % (int)_selectedCurve->dataSize();
//
//     if (index != _selectedPoint)
//     {
//         _selectedPoint = index;
//     }
//     emit markerSelectionChanged();
}
