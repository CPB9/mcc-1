#pragma once
#include "mcc/Config.h"
#include "mcc/msg/Objects.h"
#include "mcc/msg/obj/Protocol.h"
#include <bmcl/SharedBytes.h>
#include <string>

namespace mccmsg {


class MCC_MSG_DECLSPEC DeviceUiDescriptionObj : public mcc::RefCountable
{
public:
    DeviceUiDescriptionObj(const DeviceUi& name, const ProtocolValue& id, const bmcl::SharedBytes& data);
    DeviceUiDescriptionObj(const DeviceUi& name, ProtocolValue&& id, const bmcl::SharedBytes& data);
    ~DeviceUiDescriptionObj() override;
    const DeviceUi& name() const;
    const ProtocolValue& id() const;
    const bmcl::SharedBytes& data() const;
private:
    DeviceUi _name;
    ProtocolValue _id;
    bmcl::SharedBytes _data;
};

using DeviceUis = std::vector<DeviceUi>;
using DeviceUiDescription = bmcl::Rc<const DeviceUiDescriptionObj>;
using DeviceUiDescriptions = std::vector<DeviceUiDescription>;
}

