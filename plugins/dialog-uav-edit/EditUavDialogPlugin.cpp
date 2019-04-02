#include "mcc/plugin/Plugin.h"
#include "mcc/plugin/PluginCache.h"

#include "EditUavDialog.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/uav/UavController.h"
#include "mcc/ui/UiPlugin.h"

#include <bmcl/OptionUtils.h>

#include <QAction>

using namespace mccuav;
using namespace mccui;

class EditUavDialogPlugin : public UiPlugin {
public:
    EditUavDialogPlugin()
        : UiPlugin("edituavdialog")
        , _dialog(nullptr)
    {
    }

    ~EditUavDialogPlugin() override
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
        auto actionsData = cache->findPluginData<GlobalActionsPluginData>();
        if (bmcl::anyNone(uavData, actionsData)) {
            return;
        }

        _dialog = new EditUavDialog(uavData->uavController());

        QObject::connect(actionsData->globalActions()->showEditUavDialogAction(), &QAction::triggered, _dialog,
                         [this, actionsData]()
        {
            QVariant data = actionsData->globalActions()->showEditUavDialogAction()->data();
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

    EditUavDialog* _dialog;
};

static void create(mccplugin::PluginCacheWriter* cache)
{
    cache->addPlugin(std::make_shared<EditUavDialogPlugin>());
}

MCC_INIT_PLUGIN(create);

