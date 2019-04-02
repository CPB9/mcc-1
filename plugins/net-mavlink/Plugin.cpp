#include "Firmware.h"
#include "device/Tm.h"

#include "broker/Broker.h"
#include "widgets/FirmwareWidget.h"
#include "widgets/MavlinkToolbarWidget.h"

#include <QFile>
#include <caf/actor.hpp>
#include <caf/scheduled_actor.hpp>
#include <bmcl/StringView.h>
#include <bmcl/Logging.h>
#include <bmcl/MakeRc.h>

#include "mcc/plugin/Plugin.h"
#include "mcc/plugin/PluginCache.h"
#include "mcc/uav/FirmwareWidgetPlugin.h"
#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/UavController.h"
#include "mcc/net/NetPlugin.h"
#include "mcc/ui/WidgetPlugin.h"

#include <bmcl/OptionUtils.h>

class MavlinkWidgetPlugin : public mccuav::FirmwareWidgetPlugin {
public:
    explicit MavlinkWidgetPlugin(const mccmsg::Protocol& protocol)
        : mccuav::FirmwareWidgetPlugin(protocol)
    {
    }

    bool init(mccplugin::PluginCache* cache) override
    {
        auto chanData = cache->findPluginData<mccuav::ChannelsControllerPluginData>();
        auto uavData = cache->findPluginData<mccuav::UavControllerPluginData>();
        if (bmcl::allSome(chanData, uavData)) {
            setWidget(new mccmav::FirmwareWidget(protocol(), chanData->channelsController(), uavData->uavController()));
            return true;
        }
        return false;
    }
};

class ToolbarWidgetPlugin : public mccui::ToolBarPlugin {
public:
    explicit ToolbarWidgetPlugin(const mccmsg::Protocol& protocol)
    {
        _protocol = protocol;
    }

    bool init(mccplugin::PluginCache* cache) override
    {
         auto uavData = cache->findPluginData<mccuav::UavControllerPluginData>();
         if (bmcl::anyNone(uavData)) {
             return false;
         }

        setWidget(new mccmav::MavlinkToolbarWidget(uavData->uavController(), _protocol));

        return true;
    }

    Qt::Alignment alignment() const override
    {
        return Qt::AlignLeft;
    }
private:
    mccmsg::Protocol _protocol;
};

class MavNetPlugin : public mccnet::NetPlugin
{
public:
    MavNetPlugin(const mccmsg::ProtocolDescription& d) : mccnet::NetPlugin(d)
    {
    }
    ~MavNetPlugin() override
    {
    }
    mccnet::FrmCreator getFirmwareCreator() const override
    {
        auto r = [](const mccmsg::ProtocolValue& id, bmcl::Bytes bytes) -> bmcl::OptionRc<const mccmsg::IFirmware>
        {
            auto r = mccmav::Firmware::decode(id, bytes);
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
            return caf::actor_cast<caf::actor>(spawner->spawn<mccmav::Broker, caf::monitored>(core, logger, group, d));
        };
        return r;
    }
    mccnet::TmStorCreator getTmStorageCreator() const override
    {
        auto r = [](const bmcl::OptionRc<const mccmsg::ITmView>& view) -> bmcl::OptionRc<mccmsg::ITmStorage>
        {
            auto v = bmcl::dynamic_pointer_cast<const mccmav::TmView>(view.unwrapOr(nullptr));
            if (v.isNull()) return bmcl::None;
            return (mccmsg::ITmStoragePtr)bmcl::makeRc<mccmav::TmStorage>(v);
        };
        return r;
    }

private:
};

void create(mccplugin::PluginCacheWriter* cache)
{
    QFile file(":/net-mavlink/icon.svg");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        BMCL_WARNING() << "Не удалось открыть файл с иконкой протокола обмена mavlink: " << file.fileName();
        return;
    }
    QByteArray bytes = file.readAll();
    bmcl::Buffer icon(bytes.data(), bytes.size());

    auto p = mccmsg::Protocol::createOrNil("{86d8a804-c74a-4108-afa6-f1fbeaae4880}");
    auto dscr = bmcl::makeRc<mccmsg::ProtocolDescriptionObj>(p, true, true, std::chrono::milliseconds(10), "mavlink", "Идентификатор Mavlink", std::move(icon), mccmsg::PropertyDescriptionPtrs(), mccmsg::PropertyDescriptionPtrs());

    cache->addPlugin(std::make_shared<MavlinkWidgetPlugin>(p));
    cache->addPlugin(std::make_shared<ToolbarWidgetPlugin>(p));
    cache->addPlugin(std::make_shared<MavNetPlugin>(dscr));
}

MCC_INIT_PLUGIN(create)
