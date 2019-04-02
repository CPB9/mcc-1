#include "mcc/plugin/Plugin.h"
#include "mcc/plugin/PluginCache.h"

#include "RemoveUavDialog.h"
#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/uav/UavController.h"
#include "mcc/ui/UiPlugin.h"

#include <bmcl/OptionUtils.h>

#include <QAction>

using namespace mccuav;
using namespace mccui;

class RemoveUavDialogPlugin : public UiPlugin {
public:
    RemoveUavDialogPlugin()
        : UiPlugin("removeuavdialog")
        , _dialog(nullptr)
    {
    }

    ~RemoveUavDialogPlugin() override
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
        auto actionsData = cache->findPluginData<GlobalActionsPluginData>();
        if (bmcl::anyNone(uavData, actionsData, channelsData)) {
            return;
        }

        _dialog = new RemoveUavDialog(uavData->uavController(), channelsData->channelsController());

        QObject::connect(actionsData->globalActions()->showRemoveUavDialogAction(), &QAction::triggered, _dialog,
                         [this, actionsData]()
        {
            QVariant data = actionsData->globalActions()->showRemoveUavDialogAction()->data();
            if(data.isValid() && data.canConvert<QUuid>())
            {
                _dialog->setUav(mccmsg::Device(data.toUuid()));
                _dialog->show();
            }
        });
    }

    void setMccMainWindow(QWidget* parent) override
    {
        if (_dialog) {
            //_dialog->setParent(parent); //HACK: не работает
        }
    }

    RemoveUavDialog* _dialog;
};

static void create(mccplugin::PluginCacheWriter* cache)
{
    cache->addPlugin(std::make_shared<RemoveUavDialogPlugin>());
}

MCC_INIT_PLUGIN(create);

