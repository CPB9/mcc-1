#pragma once

#include "mcc/Config.h"

#include <map>
#include <QString>
#include <QVector>

#include <qwt_plot.h>

#include "mcc/uav/PlotData.h"
#include "mcc/uav/Rc.h"
#include "mcc/uav/Fwd.h"


class QwtLegend;
class QwtPlotCurve;

class TmParamTreeItem;
class Curve;

class PlotWidget : public QwtPlot
{
    Q_OBJECT
public:
    explicit PlotWidget(mccuav::UavController* uavController, QWidget* parent = 0);
    ~PlotWidget();

    void timerEvent(QTimerEvent*) override;

    void addCurve(const QString& curve, const QString& title, const QColor& color, const mccuav::PlotData& var);
    void removeCurve(const QString& curve);
    void clearCurves();

    void setInterval(double secs);
    bool autoUpdate() const;

    std::vector<Curve*> curves() const;
    void showCurveContextMenu(QwtPlotCurve* curve, const QPoint& pos);
public slots:
    void curveValueChanged(const QString& curve, double x, double y);
    void setAutoUpdate(bool update);
    void legendChecked(const QVariant &itemInfo, bool on, int index);
    void showCurve(QwtPlotItem *item, bool on);

private:
    mccuav::Rc<mccuav::UavController> _uavController;
    std::map<QString, Curve*> _curves;
    QwtLegend* _legend;
    double _interval;
    bool _isAutoUpdate;
};
