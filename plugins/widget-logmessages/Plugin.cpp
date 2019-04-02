#include "EventViewer.h"
#include "MessageListTool.h"
#include "mcc/plugin/PluginCache.h"
#include "mcc/uav/UavController.h"
#include "mcc/ui/WidgetPlugin.h"

using namespace mccui;
using namespace mccide;

class EventViewLabelPlugin : public ToolBarPlugin {
public:
    bool init(mccplugin::PluginCache*) override
    {
        return hasWidget();
    }

    virtual int64_t priority() const override
    {
        return 2;
    }

};

class MessageToolPlugin : public DockWidgetPlugin {
public:
    MessageToolPlugin()
        : eventViewPlugin(std::make_shared<EventViewLabelPlugin>())
    {
    }

    bool init(mccplugin::PluginCache* cache) override
    {
        auto uavData = cache->findPluginData<mccuav::UavControllerPluginData>();
        if (uavData.isNone()) {
            return false;
        }
        auto messagesTool = new MessageListTool(uavData->uavController());
        setWidget(messagesTool);

        auto eventsViewLabel = new EventViewer(messagesTool->logModel());
        eventViewPlugin->setWidget(eventsViewLabel);

        QObject::connect(eventsViewLabel, &EventViewer::showDetails, [this]() {
            if (hasWidget()) {
                showRequestCallback()(widget().unwrap());
            }
        });
        return true;
    }

    Qt::Alignment alignment() const override
    {
        return Qt::AlignBottom;
    }

    std::shared_ptr<EventViewLabelPlugin> eventViewPlugin;
};

static void create(mccplugin::PluginCacheWriter* cache)
{
    auto p = std::make_shared<MessageToolPlugin>();
    cache->addPlugin(p);
    cache->addPlugin(p->eventViewPlugin);
}

MCC_INIT_PLUGIN(create);
