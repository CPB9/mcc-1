#include "CafService.h"

#include "mcc/plugin/PluginCache.h"
#include "mcc/uav/ExchangeService.h"
#include "mcc/net/NetProxy.h"
#include "mcc/net/NetPlugin.h"
#include "mcc/msg/ProtocolController.h"
#include <bmcl/MakeRc.h>

class CafPlugin : public mccplugin::Plugin {
public:
    explicit CafPlugin(CafService* service, bmcl::Option<uint16_t> maxThreads)
        : mccplugin::Plugin("cafplugin$_^dummy*_&plugin")
        , _service(service)
        , _proxy(true, maxThreads)
    {
        _proxy.setBmclLogHandler();
        _proxy.setQtLogHandler();
        service->setCore(_proxy.core());
    }
    bool init(mccplugin::PluginCache* cache) override
    {
        _proxy.loadPlugins(cache);
        return true;
    }
    void postInit(mccplugin::PluginCache* cache) override
    {
    }

    bmcl::Rc<CafService> _service;
    mccnet::NetProxy _proxy;
};

static void create(mccplugin::PluginCacheWriter* cache)
{
    bmcl::Rc<CafService> service = new CafService;
    bmcl::Option<uint16_t> maxThreads = 1;
    cache->addPluginData(std::make_unique<mccuav::ExchangeServicePluginData>(service.get()));
    cache->addPlugin(std::make_shared<CafPlugin>(service.get(), maxThreads));
}

MCC_INIT_PLUGIN(create);
