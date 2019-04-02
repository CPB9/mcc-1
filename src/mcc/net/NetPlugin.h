#pragma once
#include "mcc/Config.h"
#include <functional>
#include "mcc/plugin/Plugin.h"
#include "mcc/msg/obj/Firmware.h"
#include "mcc/msg/obj/Protocol.h"
#include "mcc/msg/TmView.h"

namespace caf { class actor; }
namespace caf { class scheduled_actor; }

namespace mccnet {

using ProtCreator = std::function<caf::actor(caf::scheduled_actor* spawner, const caf::actor& core, const caf::actor& logger, const caf::actor& group)>;
using FrmCreator = std::function<bmcl::OptionRc<const mccmsg::IFirmware>(const mccmsg::ProtocolValue& id, bmcl::Bytes bytes)>;
using TmStorCreator = std::function<bmcl::OptionRc<mccmsg::ITmStorage>(const bmcl::OptionRc<const mccmsg::ITmView>& view)>;

class MCC_PLUGIN_NET_DECLSPEC NetPlugin : public mccplugin::Plugin
{
public:
    static constexpr const char* id = "mccnet.plugin";
    NetPlugin(const mccmsg::ProtocolDescription&);
    ~NetPlugin() override;
    const mccmsg::Protocol& protocol() const;
    const mccmsg::ProtocolDescription& description() const;
    virtual FrmCreator getFirmwareCreator() const = 0;
    virtual ProtCreator getProtocolCreator() const = 0;
    virtual TmStorCreator getTmStorageCreator() const = 0;
    virtual mccmsg::PropertyDescriptionPtrs getOptionalProperties() const;
    virtual mccmsg::PropertyDescriptionPtrs getRequiredProperties() const;
private:
    mccmsg::ProtocolDescription _description;
};

}
