#include "CafService.h"

#include "mcc/plugin/PluginCache.h"
#include "mcc/ui/Settings.h"
#include "mcc/uav/ExchangeService.h"
#include "mcc/net/NetProxy.h"
#include "mcc/net/NetPlugin.h"
#include "mcc/msg/ProtocolController.h"
#include <bmcl/MakeRc.h>
#include <caf/defaults.hpp>

class CafPlugin : public mccplugin::Plugin {
public:
    explicit CafPlugin() : mccplugin::Plugin("cafplugin$_^dummy*_&plugin")
    {
    }
    ~CafPlugin() override
    {
        if (_settingsPort.isSome())
            _portWriter->write(_settingsPort.unwrap());
        else
            _portWriter->write(QVariant());

        if (_settingsHost.isSome() && !_settingsHost->empty())
            _hostWriter->write(QString::fromStdString(_settingsHost.unwrap()));
        else
            _hostWriter->write(QVariant());

        if (_settingsMaxThreads.isSome())
             _threadsWriter->write(_settingsMaxThreads.unwrap());
        else
            _threadsWriter->write(QVariant());
    }
    bool init(mccplugin::PluginCache* cache) override
    {
        auto settingsData = cache->findPluginData<mccui::SettingsPluginData>();
        if (settingsData.isNone()) {
            return false;
        }
        mccui::Settings* settings = settingsData->settings();
        _portWriter = settings->acquireUniqueWriter("caf/port").unwrap();
        _hostWriter = settings->acquireUniqueWriter("caf/host").unwrap();
        _threadsWriter = settings->acquireUniqueWriter("caf/threads"/*, caf::defaults::scheduler::max_threads*/).unwrap();

        QVariant threads = _threadsWriter->read();
        if (!threads.isNull())
        {
            _settingsMaxThreads = threads.toUInt();
            if (_settingsMaxThreads.unwrap() == 0)
                _settingsMaxThreads.clear();
        }

        QVariant sPort = _portWriter->read();
        if (!sPort.isNull())
            _settingsPort = sPort.toUInt();

        QVariant sHost = _hostWriter->read();
        if (!sHost.isNull())
            _settingsHost = sHost.toString().toStdString();

        if (_settingsPort.isNone() || _settingsHost.isNone())
        {
            _proxy = new mccnet::NetProxy(true, _settingsMaxThreads, cache, _settingsPort);// здесь должна быть подгрузка типов сообщений, поэтому loadPlugins должен вызываться из конструктора NetProxy
        }
        else
        {
            _proxy = new mccnet::NetProxy(true, _settingsMaxThreads, cache, _settingsHost.unwrap(), _settingsPort.unwrap());
        }

        _service = new CafService;
        _service->setCore(_proxy->core());
        cache->addPluginData(std::make_unique<mccuav::ExchangeServicePluginData>(_service.get()));
        return true;
    }
    void postInit(mccplugin::PluginCache* cache) override
    {
    }

    bmcl::Option<uint16_t>      _settingsMaxThreads;
    bmcl::Option<uint16_t>      _settingsPort;
    bmcl::Option<std::string>   _settingsHost;

    bmcl::Rc<CafService> _service;
    bmcl::Rc<mccnet::NetProxy> _proxy;
    mccui::Rc<mccui::SettingsWriter> _portWriter;
    mccui::Rc<mccui::SettingsWriter> _hostWriter;
    mccui::Rc<mccui::SettingsWriter> _threadsWriter;
};

static void create(mccplugin::PluginCacheWriter* cache)
{
    cache->addPlugin(std::make_shared<CafPlugin>());
}

MCC_INIT_PLUGIN(create);
