#include "mcc/plugin/Plugin.h"
#include "mcc/plugin/PluginCache.h"

#include "mcc/ui/CoordinateSystemController.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/uav/GroupsController.h"
#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/RoutesController.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/UavUiController.h"
#include "mcc/uav/UavsTracker.h"
#include "mcc/uav/PlotController.h"
#include "mcc/ui/Settings.h"
#include "mcc/uav/ExchangeService.h"
#include "mcc/hm/HmStackReader.h"
#include "mcc/ui/HeightmapController.h"
#include "mcc/ui/UserNotifier.h"
#include "mcc/geo/Constants.h"
#include "mcc/msg/ProtocolController.h"

#include <QApplication>

using mccui::Rc;

class UiDummyPlugin : public mccplugin::Plugin {
public:
    UiDummyPlugin()
        : mccplugin::Plugin("&dummy_ui=plugin")
        , _baseInitialized(false)
    {
    }

    bool init(mccplugin::PluginCache* cache) override
    {
        if (!_baseInitialized) {
            _settings = new mccui::Settings;
            _csController = new mccui::CoordinateSystemController(_settings.get());
            _userNotifier = new mccui::UserNotifier(_settings.get());
            _actions = new mccuav::GlobalActions;
            _routesController = new mccuav::RoutesController;
            _plotController = new mccuav::PlotController(_settings.get());
            QVariant vpath = _settings->read("map/heightMapCachePath");
            QString path;
            if (!vpath.isValid()) {
                path = QApplication::applicationDirPath() + "/heightmap";
                _settings->tryWrite("map/heightMapCachePath", path);
            } else {
                path = vpath.toString();
            }

            Rc<mcchm::RcGeod> wgs84Geod = new mcchm::RcGeod(mccgeo::wgs84a<double>(), mccgeo::wgs84f<double>());
            _hmReader = new mcchm::HmStackReader(wgs84Geod.get());
            _hmController = new mccui::HeightmapController(_hmReader.get());

            cache->addPluginData(bmcl::makeUnique<mccui::SettingsPluginData>(_settings.get()));
            cache->addPluginData(bmcl::makeUnique<mccui::CoordinateSystemControllerPluginData>(_csController.get()));
            cache->addPluginData(bmcl::makeUnique<mccuav::GlobalActionsPluginData>(_actions.get()));
            cache->addPluginData(bmcl::makeUnique<mccuav::RoutesControllerPluginData>(_routesController.get()));
            cache->addPluginData(bmcl::makeUnique<mccui::HmControllerPluginData>(_hmController.get()));
            cache->addPluginData(bmcl::makeUnique<mccui::UserNotifierPluginData>(_userNotifier.get()));
            cache->addPluginData(bmcl::makeUnique<mccuav::PlotControllerPluginData>(_plotController.get()));
            _baseInitialized = true;
        }

        auto exchangeData = cache->findPluginData<mccuav::ExchangeServicePluginData>();
        if (exchangeData.isNone()) {
            return false;
        }
        mccuav::ExchangeService* service = exchangeData->service();

        auto pc = cache->findPluginData<mccmsg::ProtocolControllerPluginData>();
        if (pc.isNone())
            return false;

        Rc<mccuav::ChannelsController> chanController = new mccuav::ChannelsController(_settings.get(), service);

        _uavController = new mccuav::UavController(_settings.get(),
                                                   chanController.get(),
                                                   _routesController.get(),
                                                   _hmController.get(),
                                                   pc->controller(),
                                                   service);
        Rc<mccuav::GroupsController> groupsController = new mccuav::GroupsController(_uavController.get(), service);
        Rc<mccuav::UavsTracker> uavsTracker = new mccuav::UavsTracker(_uavController.get(), _settings.get());
        Rc<mccuav::UavUiController> uiController = new mccuav::UavUiController(_settings.get(), _uavController.get(), service);

        cache->addPluginData(std::make_unique<mccuav::GroupsControllerPluginData>(groupsController.get()));
        cache->addPluginData(std::make_unique<mccuav::ChannelsControllerPluginData>(chanController.get()));
        cache->addPluginData(std::make_unique<mccuav::UavControllerPluginData>(_uavController.get()));
        cache->addPluginData(std::make_unique<mccuav::UavsTrackerPluginData>(uavsTracker.get()));
        cache->addPluginData(std::make_unique<mccuav::UavUiControllerPluginData>(uiController.get()));
        return true;
    }

    void postInit(mccplugin::PluginCache* cache) override
    {
        _uavController->init();
    }

private:
    mccui::Rc<mccui::Settings> _settings;
    mccui::Rc<mccui::CoordinateSystemController> _csController;
    Rc<mccuav::GlobalActions> _actions;
    Rc<mccuav::RoutesController> _routesController;
    Rc<mccuav::UavController> _uavController;
    Rc<mccuav::UavUiController> _uiController;
    Rc<mcchm::HmStackReader> _hmReader;
    Rc<mccui::HeightmapController> _hmController;
    Rc<mccui::UserNotifier> _userNotifier;
    Rc<mccuav::PlotController> _plotController;
    bool _baseInitialized;
};

static void create(mccplugin::PluginCacheWriter* cache)
{
    cache->addPlugin(std::make_shared<UiDummyPlugin>());
}

MCC_INIT_PLUGIN(create);
