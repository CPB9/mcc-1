#include <bmcl/StringView.h>
#include "mcc/msg/obj/DeviceUi.h"
#include "mcc/msg/obj/Protocol.h"

namespace mccmsg {

DeviceUiDescriptionObj::DeviceUiDescriptionObj(const DeviceUi& name, const ProtocolValue& id, const bmcl::SharedBytes& data)
    : _name(name), _id(id), _data(data) {}
DeviceUiDescriptionObj::DeviceUiDescriptionObj(const DeviceUi& name, ProtocolValue&& id, const bmcl::SharedBytes& data)
    : _name(name), _id(std::move(id)), _data(data) {}
DeviceUiDescriptionObj::~DeviceUiDescriptionObj() {}
const DeviceUi& DeviceUiDescriptionObj::name() const { return _name; }
const ProtocolValue& DeviceUiDescriptionObj::id() const { return _id; }
const bmcl::SharedBytes& DeviceUiDescriptionObj::data() const { return _data; }

}
