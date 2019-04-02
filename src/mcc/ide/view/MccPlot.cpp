#include "mcc/ide/view/MccPlot.h"
#include "mcc/uav/Structs.h"

#include "mcc/uav/UavController.h"
#include "mcc/uav/Uav.h"

#include <bmcl/DoubleEq.h>

#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_legend_label.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_legenditem.h>
#include <qwt_plot_canvas.h>

#include <QDateTime>
#include <QTimerEvent>

namespace mccide {

MccPlot::~MccPlot()
{
}

class TimeScaleDraw : public QwtScaleDraw
{
public:
    QwtText label(double v) const override
    {
        QDateTime value = QDateTime::fromMSecsSinceEpoch((qint64)v);
        return value.toString("hh:mm:ss");
    }
};


Curve::Curve(MccPlot* plot, const QString& name, const QString& title, const mccuav::PlotData& var)
    : _var(var)
    , _multiplier(1.0)
    , _name(name)
    , _offset(0.0)
{
    _curve = new QwtPlotCurve(title);
    _curve->setRenderHint(QwtPlotItem::RenderAntialiased);

    _curve->attach(plot);
}

void Curve::addPoint(double x, double y)
{
    if (!_x.empty() && bmcl::doubleEq(*(_x.end() - 1), x))
        return;

    _x.append(x);
    _y.append(y);

    _curve->setRawSamples(_x.data(), _y.data(), _x.size());
}

void Curve::tick(double x)
{
    if (_x.empty())
        return;

    addPoint(x, _y.last());
}

void Curve::cut(double xInterval)
{
    if (_x.empty())
        return;

    QDateTime value = QDateTime::currentDateTime();

    double msecsSinceEpoch = (double)value.toMSecsSinceEpoch();

    double minTime = msecsSinceEpoch - xInterval - 1000;

    while (_x.size() > 2 && (_x.first() < minTime))
    {
        _x.removeFirst();
        _y.removeFirst();
    }
}

void Curve::remove()
{
    _curve->detach();
}

QString Curve::name() const
{
    return _name;
}

mccuav::PlotData Curve::variable() const
{
    return _var;
}

QColor Curve::color() const
{
    return _curve->pen().color();
}

double Curve::multiplier() const
{
    return _multiplier;
}

double Curve::offset() const
{
    return _offset;
}

QwtPlotCurve* Curve::qwtCurve() const
{
    return _curve;
}

void Curve::setColor(QColor color)
{
    _curve->setPen(color, 2);
}

MccPlot::MccPlot(mccuav::UavController* uavController, QWidget* parent /*= 0*/)
    : QwtPlot(parent)
    , _uavController(uavController)
{
    QwtPlotCanvas *canvas = new QwtPlotCanvas();
    canvas->setFocusIndicator(QwtPlotCanvas::CanvasFocusIndicator);
    canvas->setFocusPolicy(Qt::StrongFocus);
    setCanvas(canvas);

    setAutoReplot(false);

    setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw());
    setAxisLabelRotation(QwtPlot::xBottom, -50.0);
    setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

     setMinimumHeight(100);
     setMinimumWidth(100);

    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->setMajorPen(Qt::gray, 0, Qt::DotLine);
    grid->setMinorPen(Qt::darkGray, 0, Qt::DotLine);
    grid->attach(this);

    _legend = new QwtLegend();
    _legend->setDefaultItemMode(QwtLegendData::Checkable);
    _legend->show();
    insertLegend(_legend, QwtPlot::LeftLegend);

    connect(_legend, &QwtLegend::checked, this, &MccPlot::legendChecked);

    setInterval(10);
}

void MccPlot::timerEvent(QTimerEvent* event)
{
    if (!_isAutoUpdate)
    {
        killTimer(event->timerId());
        return;
    }

    for (auto& c : _curves.values())
    {
        c->tick(QDateTime::currentMSecsSinceEpoch());
    }

    replot();
}

void MccPlot::curveValueChanged(const QString& curve, double x, double y)
{
    if (!_curves.contains(curve))
        return;

    _curves[curve]->addPoint(x, y);

    for (auto& c : _curves.values())
    {
        c->cut(_interval);
    }

    quint64 currentMsecs = QDateTime::currentMSecsSinceEpoch();

    setAxisScale(QwtPlot::xBottom, (double)currentMsecs - (double)_interval, (double)currentMsecs);

    replot();
}

void MccPlot::addCurve(const QString& name, const QString& title, const QColor& color, const mccuav::PlotData& var)
{
    auto device = _uavController->uav(var.device());
    QString deviceInfo = device.isSome() ? device->getInfo() : "unknown";
    QString fullPlotName = QString("[%1]%2").arg(deviceInfo).arg(name);
    QString fullPlotTitle = QString("[%1]%2").arg(deviceInfo).arg(title);

    auto curve = new Curve(this, fullPlotName, fullPlotTitle, var);
    _curves[fullPlotName] = curve;
    curve->setColor(color);
    showCurve(curve->qwtCurve(), true);
    replot();
}

void MccPlot::removeCurve(const QString& title)
{
    _curves[title]->remove();
    _curves.remove(title);
    replot();
}

void MccPlot::clearCurves()
{
    for (auto p : _curves)
    {
        p->remove();
    }
    _curves.clear();
    replot();
}

void MccPlot::setInterval(double secs)
{
    _interval = secs * 1000.0;
}

bool MccPlot::autoUpdate() const
{
    return _isAutoUpdate;
}

QList<Curve*> MccPlot::curves() const
{
    return _curves.values();
}

void MccPlot::setAutoUpdate(bool update)
{
    _isAutoUpdate = update;

    if (_isAutoUpdate)
        startTimer(100);
}

void MccPlot::legendChecked(const QVariant &itemInfo, bool on, int index)
{
    QwtPlotItem *plotItem = infoToItem(itemInfo);
    if (plotItem)
        showCurve(plotItem, on);
}

void MccPlot::showCurve(QwtPlotItem *item, bool on)
{
    item->setVisible(on);

    QwtLegend *lgd = qobject_cast<QwtLegend *>(legend());

    QList<QWidget *> legendWidgets =
        lgd->legendWidgets(itemToInfo(item));

    if (legendWidgets.size() == 1)
    {
        QwtLegendLabel *legendLabel =
            qobject_cast<QwtLegendLabel *>(legendWidgets[0]);

        if (legendLabel)
            legendLabel->setChecked(on);
    }

    replot();
}
}
