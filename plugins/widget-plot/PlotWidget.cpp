#include "PlotWidget.h"
#include "PlotCurve.h"

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
#include <QMouseEvent>
#include <QMenu>

#include "fmt/format.h"

class LegendPicker : public QObject
{
public:
    explicit LegendPicker(PlotWidget* plot, QwtPlotCurve* curve, QwtLegendLabel* label)
        : QObject(plot)
    {
        m_pLegendLabel = label;
        m_pCurve = curve;
        if(m_pLegendLabel)
        {
            m_pLegendLabel->installEventFilter(this);
        }
    }

    ~LegendPicker()
    {
    }
private:
    QwtPlotCurve *m_pCurve;
    QwtLegendLabel *m_pLegendLabel;
    PlotWidget* plot()
    {
        return qobject_cast<PlotWidget*>(parent());
    }

    bool eventFilter(QObject* p_object, QEvent* p_event)
    {
        if(p_object == (QObject*)m_pLegendLabel)
        {
            switch(p_event->type())
            {
            case QEvent::ContextMenu:
            {
                 QMouseEvent* p_mouse_event = static_cast<QMouseEvent*>(p_event);
                 plot()->showCurveContextMenu(m_pCurve, m_pLegendLabel->mapToGlobal(p_mouse_event->pos()));
                break;
            }
            default:
                break;
            }
        }
        return QObject::eventFilter(p_object, p_event);
    }
};


class TimeScaleDraw : public QwtScaleDraw
{
public:
    QwtText label(double v) const override
    {
        QDateTime value = QDateTime::fromMSecsSinceEpoch((qint64)v);
        return value.toString("hh:mm:ss");
    }
};

PlotWidget::PlotWidget(mccuav::UavController* uavController, QWidget* parent /*= 0*/)
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

    connect(_legend, &QwtLegend::checked, this, &PlotWidget::legendChecked);

    setInterval(10);
}

PlotWidget::~PlotWidget()
{
    clearCurves();
}

void PlotWidget::timerEvent(QTimerEvent* event)
{
    if (!_isAutoUpdate)
    {
        killTimer(event->timerId());
        return;
    }

    if(_curves.empty())
        return;

    for (auto& c : _curves)
    {
        c.second->tick(QDateTime::currentMSecsSinceEpoch());
    }

    quint64 currentMsecs = QDateTime::currentMSecsSinceEpoch();
    setAxisScale(QwtPlot::xBottom, (double)currentMsecs - (double)_interval, (double)currentMsecs);
    replot();
}

void PlotWidget::curveValueChanged(const QString& curve, double x, double y)
{
    auto it = _curves.find(curve);
    if (it == _curves.end())
        return;

    it->second ->addPoint(x, y);

    for (auto& c : _curves)
    {
        c.second->cut(_interval);
    }
//    replot();
}

void PlotWidget::addCurve(const QString& name, const QString& title, const QColor& color, const mccuav::PlotData& var)
{
    auto device = _uavController->uav(var.device());
    QString deviceInfo = device.isSome() ? device->getInfo() : "unknown";
    QString fullPlotName = QString("[%1]%2").arg(deviceInfo).arg(name);
    QString fullPlotTitle = QString("[%1]%2").arg(deviceInfo).arg(title);
    auto storage = device->tmStorage();
    if(storage.isNull())
    {
        assert(false);
        return;
    }

    auto extension = storage->getExtension<mccmsg::INamedAccess>();
    if(extension.isNone())
    {
        assert(false);
        return;
    }
    auto curve = new Curve(this, extension.unwrap(), fullPlotName, fullPlotTitle, var);
    _curves[fullPlotName] = curve;
    curve->setColor(color);


    QwtLegend *p_plot_legend = qobject_cast<QwtLegend*>(legend());
    QVariant variant = itemToInfo(curve->qwtCurve());
    QWidget *p_legend_widget = p_plot_legend->legendWidget(variant);
    QwtLegendLabel *p_legend_label = qobject_cast<QwtLegendLabel *>(p_legend_widget);
    (void) new LegendPicker(this, curve->qwtCurve(), p_legend_label);

    showCurve(curve->qwtCurve(), true);
    replot();
}

void PlotWidget::removeCurve(const QString& title)
{
    auto c = _curves[title];
    c->remove();
    _curves.erase(title);
    delete c;
    replot();
    emit requestSave();
}

void PlotWidget::clearCurves()
{
    for (auto& p : _curves)
    {
        p.second->remove();
        delete p.second;
    }
    _curves.clear();
    replot();
}

void PlotWidget::setInterval(double secs)
{
    _interval = secs * 1000.0;
}

bool PlotWidget::autoUpdate() const
{
    return _isAutoUpdate;
}

std::vector<Curve*> PlotWidget::curves() const
{
    std::vector<Curve*> curves;
    for(const auto& c : _curves)
        curves.push_back(c.second);
    return curves;
}

void PlotWidget::showCurveContextMenu(QwtPlotCurve* curve, const QPoint& pos)
{
    QMenu menu;
    auto action = menu.addAction("Удалить");

    if(menu.exec(pos) == action)
    {
        auto it = std::find_if(_curves.begin(), _curves.end(), [curve](const std::pair<QString, Curve*>& itr) { return itr.second->qwtCurve() == curve; });
        if(it == _curves.end())
        {
            return;
        }
        removeCurve(it->first);
    }
}

void PlotWidget::setAutoUpdate(bool update)
{
    _isAutoUpdate = update;

    if (_isAutoUpdate)
        startTimer(100);
}

void PlotWidget::legendChecked(const QVariant &itemInfo, bool on, int index)
{
    QwtPlotItem *plotItem = infoToItem(itemInfo);
    if (plotItem)
        showCurve(plotItem, on);
}

void PlotWidget::showCurve(QwtPlotItem *item, bool on)
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
