#include "RouteSectionPlot.h"

#include "mcc/ui/WidgetPlugin.h"
#include "mcc/plugin/PluginCache.h"

#include "mcc/ui/Settings.h"
#include "mcc/uav/RoutesController.h"
#include "mcc/uav/UavController.h"
#include "mcc/vis/RadarGroup.h"
#include "mcc/ui/HeightmapController.h"

#include <bmcl/OptionUtils.h>

class RouteSectionWidgetPlugin : public mccui::DockWidgetPlugin {
public:
    RouteSectionWidgetPlugin()
        : _routeShape(nullptr)
    {
    }

    bool init(mccplugin::PluginCache* cache) override
    {
        auto settingsData = cache->findPluginData<mccui::SettingsPluginData>();
        auto routeData = cache->findPluginData<mccuav::RoutesControllerPluginData>();
        auto uavData = cache->findPluginData<mccuav::UavControllerPluginData>();
        if (bmcl::anyNone(settingsData, routeData, uavData)) {
            return false;
        }
        bmcl::Option<mccvis::RadarGroupPtr> radarController;

        _routeShape = new RouteSectionPlot(settingsData->settings(),
                                           routeData->routesController(),
                                           uavData->uavController(),
                                           radarController);
        setWidget(_routeShape);
        return true;
    }

    void postInit(mccplugin::PluginCache* cache) override
    {
        if (!_routeShape) {
            return;
        }
        auto radarData = cache->findPluginData<mccvis::RadarControllerPluginData>();
        bmcl::Option<mccvis::RadarGroupPtr> radarController;
        if (radarData.isSome()) {
            radarController = radarData->radarController();
        }
        _routeShape->setRadarController(radarController);

        auto hmData = cache->findPluginData<mccui::HmControllerPluginData>();
        if (hmData.isSome()) {
            _routeShape->setHeightmapController(hmData->hmReader());
        }
    }

    RouteSectionPlot* _routeShape;
};

void create(mccplugin::PluginCacheWriter* cache)
{
    cache->addPlugin(std::make_shared<RouteSectionWidgetPlugin>());
}

MCC_INIT_PLUGIN(create)
