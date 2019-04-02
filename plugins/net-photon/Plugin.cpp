#include <QFile>
#include <caf/actor.hpp>
#include <caf/scheduled_actor.hpp>
#include <bmcl/Logging.h>
#include <bmcl/MakeRc.h>

#include "mcc/plugin/Plugin.h"
#include "mcc/plugin/PluginCache.h"
#include "mcc/uav/FirmwareWidgetPlugin.h"
#include "mcc/uav/UavController.h"
#include "mcc/net/NetPlugin.h"

#include "Firmware.h"
#include "broker/Broker.h"
#include "widgets/FirmwareWidget.h"

class PhotonFirmwareWidgetPlugin : public mccuav::FirmwareWidgetPlugin {
public:
    explicit PhotonFirmwareWidgetPlugin(const mccmsg::Protocol& protocol)
        : mccuav::FirmwareWidgetPlugin(protocol)
    {
    }

    bool init(mccplugin::PluginCache* cache) override
    {
        auto uavData = cache->findPluginData<mccuav::UavControllerPluginData>();
        if (uavData.isNone()) {
            return false;
        }
        auto w = new mccphoton::FirmwareWidget(protocol(), uavData->uavController());
        setWidget(w);
        return true;
    }
};

class PhotonPlugin : public mccnet::NetPlugin
{
public:
    PhotonPlugin(const mccmsg::ProtocolDescription& dscr) : mccnet::NetPlugin(dscr)
    {
    }
    ~PhotonPlugin() override
    {
    }
    mccnet::FrmCreator getFirmwareCreator() const override
    {
        auto r = [](const mccmsg::ProtocolValue& id, bmcl::Bytes bytes) -> bmcl::OptionRc<const mccmsg::IFirmware>
        {
            auto r = mccphoton::Firmware::decode(id, bytes);
            if (r.isNone()) return bmcl::None;
            return (mccmsg::IFirmwarePtr)r.take();
        };
        return r;
    }
    mccnet::ProtCreator getProtocolCreator() const override
    {
        auto d = description();
        auto r = [d](caf::scheduled_actor* spawner, const caf::actor& core, const caf::actor& logger, const caf::actor& group) -> caf::actor
        {
            return caf::actor_cast<caf::actor>(spawner->spawn<mccphoton::Broker, caf::monitored>(core, logger, group, d));
        };
        return r;
    }
    mccnet::TmStorCreator getTmStorageCreator() const override
    {
        auto r = [](const bmcl::OptionRc<const mccmsg::ITmView>& view) -> bmcl::OptionRc<mccmsg::ITmStorage>
        {
            auto v = bmcl::dynamic_pointer_cast<const mccphoton::TmView>(view.unwrapOr(nullptr));
            if (v.isNull()) return bmcl::None;
            return (mccmsg::ITmStoragePtr)bmcl::makeRc<mccphoton::TmStorage>(v);
        };
        return r;
    }
private:
    bmcl::OptionRc<const mccmsg::ProtocolDescriptionObj> _dscr;
};

void create(mccplugin::PluginCacheWriter* cache)
{
    QFile file(":/net-photon/icon.svg");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        BMCL_WARNING() << "Не удалось открыть файл с иконкой протокола обмена photon: " << file.fileName();
        return;
    }
    QByteArray bytes = file.readAll();
    bmcl::Buffer icon(bytes.data(), bytes.size());

    auto p = mccmsg::Protocol("{304ddaa8-2481-4749-8ca9-8999b48914b3}");
    auto dscr = bmcl::makeRc<mccmsg::ProtocolDescriptionObj>(p, true, false, std::chrono::milliseconds(10), "photon", "Идентификатор Photon", std::move(icon), mccmsg::PropertyDescriptionPtrs(), mccmsg::PropertyDescriptionPtrs());
    cache->addPlugin(std::make_shared<PhotonFirmwareWidgetPlugin>(p));
    cache->addPlugin(std::make_shared<PhotonPlugin>(dscr));
}

MCC_INIT_PLUGIN(create)
