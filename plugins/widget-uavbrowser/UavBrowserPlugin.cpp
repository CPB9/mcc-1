#include "mcc/plugin/PluginCache.h"
#include "mcc/ide/view/UavBrowserTool.h"
#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/UavController.h"
#include "mcc/ui/WidgetPlugin.h"

#include <bmcl/OptionUtils.h>

class UavBrowserPlugin : public mccui::DockWidgetPlugin {
public:
    UavBrowserPlugin()
        : _widget(nullptr)
    {
    }

    bool init(mccplugin::PluginCache* cache) override
    {
        auto chanData = cache->findPluginData<mccuav::ChannelsControllerPluginData>();
        auto uavData = cache->findPluginData<mccuav::UavControllerPluginData>();
        if (bmcl::anyNone(uavData, chanData)) {
            return false;
        }

        _widget = new mccide::UavBrowserTool(chanData->channelsController(), uavData->uavController());
        setWidget(_widget);
        return true;
    }

    void postInit(mccplugin::PluginCache* cache) override
    {
        if (_widget) {
            _widget->loadPlugins(cache);
        }
    }

    Qt::Alignment alignment() const override
    {
        return Qt::AlignRight;
    }

    mccide::UavBrowserTool* _widget;
};

void create(mccplugin::PluginCacheWriter* cache)
{
    cache->addPlugin(std::make_shared<UavBrowserPlugin>());
}

MCC_INIT_PLUGIN(create);


