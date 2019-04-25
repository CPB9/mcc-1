#include "mcc/plugin/PluginCache.h"
#include "mcc/ui/WidgetPlugin.h"
#include "mcc/uav/UavController.h"
#include "mcc/ui/Settings.h"

#include "PlotTool.h"
#include <bmcl/OptionUtils.h>

class PlotWidgetPlugin : public mccui::DockWidgetPlugin {
public:
    bool init(mccplugin::PluginCache* cache) override
    {
        auto uavData = cache->findPluginData<mccuav::UavControllerPluginData>();
        auto plotData = cache->findPluginData<mccuav::PlotControllerPluginData>();
        auto settingsData = cache->findPluginData<mccui::SettingsPluginData>();
        if (bmcl::anyNone(uavData, plotData, settingsData)) {
            return false;
        }

        setWidget(new PlotTool(settingsData->settings(), uavData->uavController(), plotData->plotController()));
        return true;
    }

    Qt::Alignment alignment() const override
    {
        return Qt::AlignRight;
    }
};

void create(mccplugin::PluginCacheWriter* cache)
{
    qRegisterMetaTypeStreamOperators<mccuav::PlotData>("mccuav::PlotData");
    qRegisterMetaTypeStreamOperators<QList<mccuav::PlotData>>("QList<mccuav::PlotData>");
    cache->addPlugin(std::make_shared<PlotWidgetPlugin>());
}

MCC_INIT_PLUGIN(create);

