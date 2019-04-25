#include "PlotController.h"

namespace mccuav {

PlotController::PlotController(mccui::Settings* settings)
    : _settings(settings)
{

}

void PlotController::addPlot(const mccuav::PlotData& plotData)
{
    emit plotAdded(plotData);
    emit requestSubscription(plotData);
}

PlotControllerPluginData::PlotControllerPluginData(PlotController* plotController)
    : mccplugin::PluginData(id)
    , _plotController(plotController)
{

}

PlotControllerPluginData::~PlotControllerPluginData()
{

}

mccuav::PlotController* PlotControllerPluginData::plotController()
{
    return _plotController.get();
}

const mccuav::PlotController* PlotControllerPluginData::plotController() const
{
    return _plotController.get();
}

}
