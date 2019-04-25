#pragma once
#include <QWidget>
#include <bmcl/Option.h>
#include "mcc/msg/FwdExt.h"
#include "mcc/uav/Rc.h"
#include "mcc/uav/Fwd.h"
#include "mcc/uav/PlotController.h"

#include "mcc/ui/Settings.h"

class PlotWidget;

class PlotTool : public QWidget
{
    Q_OBJECT
public:
    PlotTool(mccui::Settings* settings, mccuav::UavController* uavController, mccuav::PlotController* plotController, QWidget* parent = 0);
    ~PlotTool();
private slots:
    void saveState();
private:
    QColor findFreeColor() const;
    void addCurve(const mccuav::PlotData& data);
private:
    mccuav::Rc<mccui::Settings> _settings;
    mccuav::Rc<mccuav::UavController> _uavController;
    mccuav::Rc<mccuav::PlotController> _plotController;
    PlotWidget* _plot;
    QList<mccuav::PlotData> _pendingCurves;
protected:
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;
};
