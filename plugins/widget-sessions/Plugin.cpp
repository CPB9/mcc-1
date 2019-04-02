#include "SessionsWidget.h"

#include "mcc/ui/WidgetPlugin.h"
#include "mcc/uav/ExchangeService.h"
#include "mcc/plugin/PluginCache.h"

#include <bmcl/OptionUtils.h>

class SessionsToolbarPlugin : public mccui::ToolBarPlugin {
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

class SessionsDockWidgetPlugin : public mccui::DockWidgetPlugin {
public:
};

class SessionsPlugin : public mccplugin::Plugin
{
public:
    SessionsPlugin(const std::shared_ptr<SessionsToolbarPlugin>& toolbar, const std::shared_ptr<SessionsDockWidgetPlugin>& dock)
        : mccplugin::Plugin("sessionsplugin$_^dummy*_&plugin"), _toolbar(toolbar), _dock(dock)
    {
    }
    bool init(mccplugin::PluginCache* cache) override
    {
        auto serviceData = cache->findPluginData<mccuav::ExchangeServicePluginData>();
        auto uavData = cache->findPluginData<mccuav::UavControllerPluginData>();
        auto channelsData = cache->findPluginData<mccuav::ChannelsControllerPluginData>();
        if (bmcl::anyNone(serviceData, uavData, channelsData)) {
            return false;
        }

        if (_dock && _toolbar)
        {
            auto p = new SessionsWidget(serviceData->service(), uavData->uavController(), channelsData->channelsController());
            //_toolbar->setWidget(p);
            _dock->setWidget(p);
            _toolbar.reset();
            _dock.reset();
        }
        return true;
    }
private:
    std::shared_ptr<SessionsToolbarPlugin> _toolbar;
    std::shared_ptr<SessionsDockWidgetPlugin> _dock;
};

static void create(mccplugin::PluginCacheWriter* cache)
{
    auto toolbar = std::make_shared<SessionsToolbarPlugin>();
    auto dock = std::make_shared<SessionsDockWidgetPlugin>();
    auto starter = std::make_shared<SessionsPlugin>(toolbar, dock);

    cache->addPlugin(toolbar);
    cache->addPlugin(dock);
    cache->addPlugin(starter);
}

MCC_INIT_PLUGIN(create);
