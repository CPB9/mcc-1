#pragma once

#include "mcc/msg/SubHolder.h"
#include "mcc/msg/exts/ITmExtension.h"
#include "mcc/msg/exts/NamedAccess.h"

#include "mcc/uav/PlotData.h"

#include <QString>
#include <QColor>
#include <QVector>

class PlotWidget;
class QwtPlotCurve;

class Curve
{
public:
    Curve(PlotWidget* plot, const bmcl::Rc<mccmsg::INamedAccess>& extension, const QString& name, const QString& title, const mccuav::PlotData& var);
    ~Curve();

    void addPoint(double x, double y);

    void cut(double xInterval);
    void tick(double x);

    void setColor(QColor color);

    void remove();

    QString name() const;
    mccuav::PlotData variable() const;
    QColor color() const;
    double multiplier() const;
    double offset() const;
    QwtPlotCurve* qwtCurve() const;
private:
    QVector<double> _x;
    QVector<double> _y;

    mccuav::PlotData _var;
    double _multiplier;
    QString _name;
    double _offset;
    QwtPlotCurve* _curve;

    bmcl::Option<mccmsg::SubHolder> _handlerId;
    bmcl::Rc<mccmsg::INamedAccess> _extension;
};

