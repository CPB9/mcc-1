#include "mcc/plugin/PluginCache.h"
#include "mcc/ui/WidgetPlugin.h"
#include "mcc/uav/UavController.h"
#include "PlotTool.h"

class PlotWidgetPlugin : public mccui::DockWidgetPlugin {
public:
    bool init(mccplugin::PluginCache* cache) override
    {
        auto uavData = cache->findPluginData<mccuav::UavControllerPluginData>();
        if (uavData.isNone()) {
            return false;
        }

        setWidget(new PlotTool(uavData->uavController()));
        return true;
    }

    Qt::Alignment alignment() const override
    {
        return Qt::AlignRight;
    }
};

void create(mccplugin::PluginCacheWriter* cache)
{
    cache->addPlugin(std::make_shared<PlotWidgetPlugin>());
}

MCC_INIT_PLUGIN(create);

