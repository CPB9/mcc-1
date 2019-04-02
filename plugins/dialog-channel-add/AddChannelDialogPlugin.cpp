#include "mcc/plugin/Plugin.h"
#include "mcc/plugin/PluginCache.h"

#include "mcc/ide/dialogs/AddChannelDialog.h"
#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/ui/UiPlugin.h"

#include <bmcl/OptionUtils.h>

#include <QAction>
#include <QUuid>
#include <QVariant>

using namespace mccuav;
using namespace mccui;

class AddChannelDummyPlugin : public UiPlugin {
public:
    AddChannelDummyPlugin()
        : UiPlugin("?addchannel_dummy")
        , _dialog(nullptr)
    {
    }

    ~AddChannelDummyPlugin() override
    {
        //HACK: setParent не работает
        if (_dialog) {
            delete _dialog;
        }
    }

    bool init(mccplugin::PluginCache*) override
    {
        return true;
    }

    void postInit(mccplugin::PluginCache* cache) override
    {
        auto channelsData = cache->findPluginData<ChannelsControllerPluginData>();
        auto actionsData = cache->findPluginData<GlobalActionsPluginData>();
        if (bmcl::anyNone(channelsData, actionsData)) {
            return;
        }

        _dialog = new mccide::AddChannelDialog(channelsData->channelsController());

        QObject::connect(actionsData->globalActions()->showAddChannelDialogAction(), &QAction::triggered,
                         _dialog, [this]()
        {
            _dialog->show(mccmsg::Channel());
        });
        QObject::connect(actionsData->globalActions()->showChannelEditDialogAction(), &QAction::triggered,
                         _dialog, [this, actionsData]()
        {
            QVariant data = actionsData->globalActions()->showChannelEditDialogAction()->data();
            if(data.isValid() && data.canConvert<QUuid>())
                _dialog->show(mccmsg::Channel(data.toUuid()));
        });
    }

    void setMccMainWindow(QWidget* parent) override
    {
        if (_dialog) {
            //_dialog->setParent(parent); //HACK: не работает
        }
    }

    mccide::AddChannelDialog* _dialog;
};

static void create(mccplugin::PluginCacheWriter* cache)
{
    cache->addPlugin(std::make_shared<AddChannelDummyPlugin>());
}

MCC_INIT_PLUGIN(create);

