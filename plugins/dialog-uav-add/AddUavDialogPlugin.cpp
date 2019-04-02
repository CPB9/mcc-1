#include "mcc/plugin/Plugin.h"
#include "mcc/plugin/PluginCache.h"

#include "mcc/ide/dialogs/AddUavDialog.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/RoutesController.h"
#include "mcc/ui/UiPlugin.h"

#include <bmcl/OptionUtils.h>

#include <QAction>

using namespace mccuav;
using namespace mccui;
using mccide::AddUavDialog;

class AddUavDummyPlugin : public UiPlugin {
public:
    AddUavDummyPlugin()
        : UiPlugin("?adduav_dummy")
        , _dialog(nullptr)
    {
    }

    ~AddUavDummyPlugin() override
    {
        //HACK: setParent не работает
        if (_dialog) {
            delete _dialog;
        }
    }

    bool init(mccplugin::PluginCache* cache) override
    {
        return true;
    }

    void postInit(mccplugin::PluginCache* cache) override
    {
        auto uavData = cache->findPluginData<UavControllerPluginData>();
        auto channelsData = cache->findPluginData<ChannelsControllerPluginData>();
        auto routesData = cache->findPluginData<RoutesControllerPluginData>();
        auto actionsData = cache->findPluginData<GlobalActionsPluginData>();
        if (bmcl::anyNone(uavData, channelsData, routesData, actionsData)) {
            return;
        }

        _dialog = new AddUavDialog(channelsData->channelsController(),
                                       routesData->routesController(),
                                       uavData->uavController());

        QObject::connect(actionsData->globalActions()->showAddUavDialogAction(), &QAction::triggered,
                         _dialog, &AddUavDialog::show);
    }

    void setMccMainWindow(QWidget* parent) override
    {
        if (_dialog) {
            //_dialog->setParent(parent); //HACK: не работает
        }
    }

    AddUavDialog* _dialog;
};

static void create(mccplugin::PluginCacheWriter* cache)
{
    cache->addPlugin(std::make_shared<AddUavDummyPlugin>());
}

MCC_INIT_PLUGIN(create);

