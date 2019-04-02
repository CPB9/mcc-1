#include "CoordinateSystemWidget.h"
#include "CoordinateSystemSettingsPage.h"
#include "CoordinateConverterTool.h"

#include "mcc/plugin/PluginCache.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/ui/CoordinateSystemController.h"
#include "mcc/ui/WidgetPlugin.h"
#include "mcc/ui/Settings.h"
#include "mcc/ui/SettingsPagePlugin.h"
#include "mcc/ui/DialogPlugin.h"

#include <bmcl/OptionUtils.h>

class CoordWidgetPlugin : public mccui::ToolBarPlugin {
public:
    bool init(mccplugin::PluginCache* cache) override
    {
        auto csControllerData = cache->findPluginData<mccui::CoordinateSystemControllerPluginData>();
        auto actionsData = cache->findPluginData<mccuav::GlobalActionsPluginData>();

        if (bmcl::anyNone(csControllerData, actionsData))
            return false;

        setWidget(new CoordinateSystemWidget(csControllerData->csController(), actionsData->globalActions()));
        return true;
    }

    Qt::Alignment alignment() const override
    {
        return Qt::AlignRight;
    }
};

class CoordSettingsPagePlugin : public mccui::SettingsPagePlugin {
public:
    explicit CoordSettingsPagePlugin(const std::shared_ptr<mccui::DialogPlugin>& p) : _converterPlugin(p)
    {
    }

    bool init(mccplugin::PluginCache* cache) override
    {
        auto settingsData = cache->findPluginData<mccui::SettingsPluginData>();
        auto csControllerData = cache->findPluginData<mccui::CoordinateSystemControllerPluginData>();
        auto actionsData = cache->findPluginData<mccuav::GlobalActionsPluginData>();
        if (bmcl::anyNone(settingsData, csControllerData, actionsData)) {
            return false;
        }

        CoordinateSettingsPage* settingsPage = new CoordinateSettingsPage(settingsData->settings());
        setSettingsPage(settingsPage);

        auto cw = new CoordinateConverterTool(csControllerData->csController(), actionsData->globalActions(), settingsPage);
        _converterPlugin->setDialog(cw);
        return true;
    }
    virtual int64_t priority() const override
    {
        return 4;
    }
private:
    std::shared_ptr<mccui::DialogPlugin> _converterPlugin;
};


void create(mccplugin::PluginCacheWriter* cache)
{
    auto converterPlugin = std::make_shared<mccui::DialogPlugin>();
    auto p = std::make_shared<CoordSettingsPagePlugin>(converterPlugin);
    cache->addPlugin(p);
    cache->addPlugin(converterPlugin);
    cache->addPlugin(std::make_shared<CoordWidgetPlugin>());
}

MCC_INIT_PLUGIN(create);
