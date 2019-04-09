#include <bmcl/StringView.h>
#include "mcc/msg/obj/Device.h"

namespace mccmsg {

DeviceDescriptionObj::~DeviceDescriptionObj()
{
}

DeviceDescriptionObj::DeviceDescriptionObj(const Device& name, const ProtocolId& protocolId)
    : _name(name)
    , _protocolId(protocolId)
    , _registerFirst(false)
    , _log(false)
{
}

DeviceDescriptionObj::DeviceDescriptionObj(const Device& name
    , bmcl::StringView info
    , bmcl::StringView settings
    , const ProtocolId& protocolId
    , const bmcl::Option<DeviceUi>& ui
    , const bmcl::SharedBytes& pixmap
    , const bmcl::Option<Firmware>& firmware
    , bool registerFirst
    , bool log)
    : _name(name)
    , _info(info.toStdString())
    , _settings(settings.toStdString())
    , _protocolId(protocolId)
    , _ui(ui)
    , _pixmap(pixmap)
    , _firmware(firmware)
    , _registerFirst(registerFirst)
    , _log(log)
{
}

const Device& DeviceDescriptionObj::name() const { return _name; }
const std::string& DeviceDescriptionObj::info() const { return _info; }
const std::string& DeviceDescriptionObj::settings() const { return _settings; }
const ProtocolId& DeviceDescriptionObj::protocolId() const { return _protocolId; }
const bmcl::SharedBytes& DeviceDescriptionObj::pixmap() const { return _pixmap; }
const bmcl::Option<DeviceUi>& DeviceDescriptionObj::ui() const { return _ui; }
const bmcl::Option<Firmware>& DeviceDescriptionObj::firmware() const { return _firmware; }
bool DeviceDescriptionObj::registerFirst() const { return _registerFirst; }
bool DeviceDescriptionObj::log() const { return _log; }

std::string DeviceDescriptionObj::getName() const
{
    if (_info.empty())
        return _name.toStdString();
    return _info;
}

}
