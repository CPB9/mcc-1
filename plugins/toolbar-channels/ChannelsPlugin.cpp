#include "mcc/plugin/PluginCache.h"
#include "mcc/ui/WidgetPlugin.h"
#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/GlobalActions.h"
#include "ChannelsWidget.h"

class ChannelsWidgetPlugin : public mccui::ToolBarPlugin {
public:
    bool init(mccplugin::PluginCache* cache) override
    {
        auto channelsData = cache->findPluginData<mccuav::ChannelsControllerPluginData>();
        auto actionsData = cache->findPluginData<mccuav::GlobalActionsPluginData>();
        if (channelsData.isNone() ||
            actionsData.isNone()) {
            return false;
        }

        setWidget(new ChannelsWidget(channelsData->channelsController(),
                                     actionsData->globalActions()));
        return true;
    }
    virtual int64_t priority() const override
    {
        return -1;
    }
};

void create(mccplugin::PluginCacheWriter* cache)
{
    cache->addPlugin(std::make_shared<ChannelsWidgetPlugin>());
}

MCC_INIT_PLUGIN(create);
