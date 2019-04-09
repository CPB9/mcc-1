#pragma once
#include "mcc/Config.h"
#include <string>
#include <bmcl/Option.h>
#include <bmcl/SharedBytes.h>
#include "mcc/msg/obj/Protocol.h"

namespace mccmsg {

class MCC_MSG_DECLSPEC DeviceDescriptionObj : public mcc::RefCountable
{
public:
    DeviceDescriptionObj(const Device& device, const ProtocolId& protocolId);
    DeviceDescriptionObj(const Device& device
        , bmcl::StringView info
        , bmcl::StringView settings
        , const ProtocolId& protocolId
        , const bmcl::Option<DeviceUi>& ui
        , const bmcl::SharedBytes& pixmap
        , const bmcl::Option<Firmware>& firmware
        , bool registerFirst
        , bool log);
    ~DeviceDescriptionObj() override;

    const Device& name() const;
    const std::string& info() const;
    const std::string& settings() const;
    const ProtocolId& protocolId() const;
    const bmcl::Option<DeviceUi>& ui() const;
    const bmcl::SharedBytes& pixmap() const;
    const bmcl::Option<Firmware>& firmware() const;
    bool registerFirst() const;
    bool log() const;

    std::string getName() const;
private:
    Device _name;
    std::string _info;
    std::string _settings;
    ProtocolId  _protocolId;
    bmcl::Option<DeviceUi> _ui;
    bmcl::SharedBytes _pixmap;
    bmcl::Option<Firmware> _firmware;
    bool _registerFirst;
    bool _log;
};

using DeviceDescription = bmcl::Rc<const DeviceDescriptionObj>;
using Devices = std::vector<Device>;
using DeviceDescriptions = std::vector<DeviceDescription>;
}
