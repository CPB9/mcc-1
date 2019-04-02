#include "StopwatchTimerTool.h"
#include "StopwatchTimerStatusWidget.h"

#include "mcc/plugin/PluginCache.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/ui/DialogPlugin.h"
#include "mcc/ui/Settings.h"
#include "mcc/ui/SettingsPagePlugin.h"
#include "mcc/ui/WidgetPlugin.h"

#include <bmcl/OptionUtils.h>

class InformatorWidgetPlugin : public mccui::ToolBarPlugin {
public:
    Qt::Alignment alignment() const override
    {
        return Qt::AlignRight;
    }
    virtual int64_t priority() const override
    {
        return -1;
    }
};

class UtilsSettingsPagePlugin : public mccplugin::Plugin {
public:
    explicit UtilsSettingsPagePlugin(const std::shared_ptr<mccui::ToolBarPlugin>& informator)
        : mccplugin::Plugin("stopwatchplugin$_^dummy*_&plugin")
        , _informatorPlugin(informator)
    {
    }

    bool init(mccplugin::PluginCache* cache) override
    {
        auto settingsData = cache->findPluginData<mccui::SettingsPluginData>();
        if (bmcl::anyNone(settingsData)) {
            return false;
        }

        if (_informatorPlugin)
        {
            _informatorPlugin->setWidget(new StopwatchTimerStatusWidget(settingsData->settings()));
            _informatorPlugin.reset();
        }
        return true;
    }
    virtual int64_t priority() const override
    {
        return 3;
    }

private:
    std::shared_ptr<mccui::ToolBarPlugin> _informatorPlugin;
};

void create(mccplugin::PluginCacheWriter* cache)
{
    auto informatorPlugin = std::make_shared<InformatorWidgetPlugin>();
    auto p = std::make_shared<UtilsSettingsPagePlugin>(informatorPlugin);

    cache->addPlugin(p);
    cache->addPlugin(informatorPlugin);
}

MCC_INIT_PLUGIN(create)
