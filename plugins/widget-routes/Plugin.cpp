#include "RouteEditorTool.h"

#include "mcc/ui/WidgetPlugin.h"
#include "mcc/plugin/PluginCache.h"

#include "mcc/ui/CoordinateSystemController.h"
#include "mcc/uav/RoutesController.h"
#include "mcc/uav/UavController.h"
#include "mcc/ui/Settings.h"
#include "mcc/msg/ProtocolController.h"

#include "mcc/map/MapRect.h"

#include <bmcl/OptionUtils.h>

class RouteEditorPlugin : public mccui::DockWidgetPlugin {
public:
    bool init(mccplugin::PluginCache* cache) override
    {
        auto settingsData = cache->findPluginData<mccui::SettingsPluginData>();
        auto csControllerData = cache->findPluginData<mccui::CoordinateSystemControllerPluginData>();
        auto routesData = cache->findPluginData<mccuav::RoutesControllerPluginData>();
        auto uavData = cache->findPluginData<mccuav::UavControllerPluginData>();
        auto rectData = cache->findPluginData<mccmap::MapRectPluginData>();
        auto pData = cache->findPluginData<mccmsg::ProtocolControllerPluginData>();

        if (bmcl::anyNone(settingsData, csControllerData, routesData, rectData, uavData, pData))
            return false;

        QWidget* r = new RouteEditorTool(csControllerData->csController(),
                                        rectData->rect(),
                                        settingsData->settings(),
                                        routesData->routesController(),
                                        uavData->uavController(),
                                        pData->controller());
        setWidget(r);
        return true;
    }
};

void create(mccplugin::PluginCacheWriter* cache)
{
    cache->addPlugin(std::make_shared<RouteEditorPlugin>());
}

MCC_INIT_PLUGIN(create)
