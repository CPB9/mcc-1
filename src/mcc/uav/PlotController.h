#pragma once

#include <QObject>

#include "mcc/ui/Fwd.h"
#include "mcc/ui/Rc.h"
#include "mcc/ui/QObjectRefCountable.h"
#include "mcc/ui/Settings.h"
#include "mcc/uav/PlotData.h"
#include "mcc/plugin/PluginData.h"

namespace mccuav {

class MCC_UAV_DECLSPEC PlotController : public mccui::QObjectRefCountable<QObject>
{
    Q_OBJECT
public:
    PlotController(mccui::Settings* settings);
    void addPlot(const mccuav::PlotData& plotData);
signals:
    void plotAdded(const mccuav::PlotData& plotData);
    void requestSubscription(const mccuav::PlotData& plotData);
private:
    mccui::Rc<mccui::Settings> _settings;
};
class MCC_UAV_DECLSPEC PlotControllerPluginData : public mccplugin::PluginData
{
public:
    static constexpr const char* id = "mccuav::PlotControllerPluginData";

    PlotControllerPluginData(PlotController* uavController);
    ~PlotControllerPluginData();

    PlotController* plotController();
    const PlotController* plotController() const;

private:
    mccui::Rc<PlotController> _plotController;
};

}
