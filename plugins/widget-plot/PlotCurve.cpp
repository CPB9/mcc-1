#include "PlotCurve.h"
#include "PlotWidget.h"

#include <fmt/format.h>

#include <qwt_plot_curve.h>
#include <qwt_legend.h>
#include <qwt_legend_label.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_legenditem.h>
#include <qwt_plot_canvas.h>

#include <QDateTime>

#include <bmcl/DoubleEq.h>
#include <bmcl/StringView.h>
#include "mcc/msg/exts/NamedAccess.h"
#include "mcc/msg/NetVariant.h"

Curve::Curve(PlotWidget* plot, const bmcl::Rc<mccmsg::INamedAccess>& extension, const QString& name, const QString& title, const mccuav::PlotData& var)
    : _var(var)
    , _multiplier(1.0)
    , _name(name)
    , _offset(0.0)
    , _extension(extension)
{
    _curve = new QwtPlotCurve(title);
    _curve->setRenderHint(QwtPlotItem::RenderAntialiased);

    _curve->attach(plot);

    std::string variableName = fmt::format("{}.{}", var.trait(), var.varId());

    mccmsg::ValueHandler handler = [this, plot, name](const mccmsg::NetVariant& value, const bmcl::SystemTime& time)
    {
        quint64 msecsToNow = bmcl::toMsecs(time.time_since_epoch()).count();
        plot->curveValueChanged(name, (double)msecsToNow, value.toDouble());
    };

    _handlerId = extension->addHandler(variableName, std::move(handler), false);
}

Curve::~Curve()
{
}

void Curve::addPoint(double x, double y)
{
    if(!_x.empty() && bmcl::doubleEq(*(_x.end() - 1), x))
        return;

    _x.append(x);
    _y.append(y);

    _curve->setRawSamples(_x.data(), _y.data(), _x.size());
}

void Curve::tick(double x)
{
    if(_x.empty())
        return;

    addPoint(x, _y.last());
}

void Curve::cut(double xInterval)
{
    if(_x.empty())
        return;

    QDateTime value = QDateTime::currentDateTime();

    double msecsSinceEpoch = (double)value.toMSecsSinceEpoch();

    double minTime = msecsSinceEpoch - xInterval - 1000;

    while(_x.size() > 2 && (_x.first() < minTime))
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

