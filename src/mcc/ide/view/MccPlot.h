#pragma once

#include "mcc/Config.h"

#include <QString>
#include <QVector>

#include <qwt_plot.h>

#include "mcc/uav/PlotData.h"
#include "mcc/uav/Rc.h"
#include "mcc/uav/Fwd.h"

class QwtLegend;
class QwtPlotCurve;

namespace mccide {

class TmParamTreeItem;
class MccPlot;

class MCC_IDE_DECLSPEC Curve
{
public:
    Curve(MccPlot* plot, const QString& name, const QString& title, const mccuav::PlotData& var);

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
};

class MCC_IDE_DECLSPEC MccPlot : public QwtPlot
{
    Q_OBJECT
public:
    explicit MccPlot(mccuav::UavController* uavController, QWidget* parent = 0);
    ~MccPlot();

    void timerEvent(QTimerEvent*) override;

    void addCurve(const QString& curve, const QString& title, const QColor& color, const mccuav::PlotData& var);
    void removeCurve(const QString& curve);
    void clearCurves();

    void setInterval(double secs);
    bool autoUpdate() const;

    QList<Curve*> curves() const;

public slots:
    void curveValueChanged(const QString& curve, double x, double y);
    void setAutoUpdate(bool update);
    void legendChecked(const QVariant &itemInfo, bool on, int index);
    void showCurve(QwtPlotItem *item, bool on);

private:
    mccuav::Rc<mccuav::UavController> _uavController;

    QMap<QString, Curve*> _curves;

    QwtLegend* _legend;

    double _interval;

    bool _isAutoUpdate;
};
}
