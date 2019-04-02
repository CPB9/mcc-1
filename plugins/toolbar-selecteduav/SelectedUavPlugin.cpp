#include "SelectedUavWidget.h"
#include "ToolbarUavSettingsPage.h"

#include "mcc/plugin/PluginCache.h"
#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/uav/GroupsController.h"
#include "mcc/uav/UavController.h"
#include "mcc/ui/Settings.h"
#include "mcc/ui/SettingsPagePlugin.h"
#include "mcc/ui/WidgetPlugin.h"

#include <bmcl/OptionUtils.h>

class SelectedUavWidgetPlugin : public mccui::ToolBarPlugin {
public:
    Qt::Alignment alignment() const override
    {
        return Qt::AlignLeft;
    }
};

class ToolbarUavSettingsPagePlugin : public mccui::SettingsPagePlugin {
public:
    explicit ToolbarUavSettingsPagePlugin()
        : _selectedUavPlugin(std::make_shared<SelectedUavWidgetPlugin>())
    {}

    bool init(mccplugin::PluginCache* cache) override
    {

        auto groupsData = cache->findPluginData<mccuav::GroupsControllerPluginData>();
        auto uavData = cache->findPluginData<mccuav::UavControllerPluginData>();
        auto actionsData = cache->findPluginData<mccuav::GlobalActionsPluginData>();
        auto settingsData = cache->findPluginData<mccui::SettingsPluginData>();
        if (bmcl::anyNone(groupsData, uavData, actionsData, settingsData))
            return false;

        ToolbarUavSettingsPage* settingsPage = new ToolbarUavSettingsPage(settingsData->settings());
        setSettingsPage(settingsPage);
        _selectedUavPlugin->setWidget(new SelectedUavWidget(groupsData->groupsController(),
                                                            uavData->uavController(),
                                                            actionsData->globalActions(),
                                                            settingsPage));
        return true;
    }
    virtual int64_t priority() const override
    {
        return 3;
    }

    void addSubPlugins(mccplugin::PluginCacheWriter* cache)
    {
        cache->addPlugin(_selectedUavPlugin);
    }

private:
    std::shared_ptr<mccui::ToolBarPlugin>       _selectedUavPlugin;
};

void create(mccplugin::PluginCacheWriter* cache)
{
    auto p = std::make_shared<ToolbarUavSettingsPagePlugin>();
    cache->addPlugin(p);
    p->addSubPlugins(cache);
}

MCC_INIT_PLUGIN(create);

