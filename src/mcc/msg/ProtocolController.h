#pragma once
#include "mcc/Config.h"
#include "mcc/Rc.h"
#include "mcc/plugin/PluginData.h"
#include "mcc/msg/obj/Firmware.h"
#include <bmcl/OptionRc.h>

namespace mccplugin { class PluginCache; }

namespace mccmsg
{

class MCC_MSG_DECLSPEC ProtocolController : public mcc::RefCountable
{
public:
    ProtocolController(mccmsg::ProtocolDescriptions&&);
    ~ProtocolController() override;
    const ProtocolDescriptions& dscrs() const;
    virtual bmcl::OptionRc<const IPropertyValue> decodeProperty(const mccmsg::Property&, bmcl::StringView) const = 0;
    virtual bmcl::OptionRc<const IFirmware> decodeFirmware(const mccmsg::ProtocolValue&, bmcl::Bytes) const = 0;
    virtual bmcl::OptionRc<ITmStorage> createStorage(const Protocol&, const ITmView*) const = 0;
private:
    ProtocolDescriptions _dscrs;
};

class MCC_MSG_DECLSPEC ProtocolControllerPluginData : public mccplugin::PluginData
{
public:
    static constexpr const char* id = "mcc::ProtocolControllerPluginData";

    ProtocolControllerPluginData(ProtocolController* settings);
    ~ProtocolControllerPluginData();
    ProtocolController* controller();
private:
    bmcl::Rc<ProtocolController> _controller;
};

}
