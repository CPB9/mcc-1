#pragma once
#include "mcc/Config.h"
#include "mcc/Rc.h"
#include "mcc/msg/obj/Protocol.h"
#include "mcc/msg/TmView.h"
#include "mcc/msg/Property.h"
#include <string>
#include <vector>

namespace bmcl { class Buffer; }

namespace mccmsg {

class MCC_MSG_DECLSPEC IFirmware : public mcc::RefCountable
{
public:
    IFirmware(const ProtocolValue& id, const PropertyDescriptionPtrs& req, const PropertyDescriptionPtrs& opt);
    virtual ~IFirmware();
    const PropertyDescriptionPtrs& required() const;
    const PropertyDescriptionPtrs& optional() const;
    const ProtocolValue& id() const;
    virtual bmcl::Buffer encode() const = 0;
private:
    ProtocolValue _id;
    PropertyDescriptionPtrs _required;
    PropertyDescriptionPtrs _optional;
};
using IFirmwarePtr = bmcl::Rc<const IFirmware>;

class MCC_MSG_DECLSPEC FirmwareDescriptionObj : public mcc::RefCountable
{
public:
    FirmwareDescriptionObj(const Firmware& name, const ProtocolValue& id, const IFirmwarePtr& frm);
    ~FirmwareDescriptionObj() override;
    const Firmware& name() const;
    const ProtocolValue& id() const;
    const IFirmwarePtr& frm() const;
private:
    Firmware      _name;
    ProtocolValue _id;
    IFirmwarePtr  _frm;
};

using Firmwares = std::vector<Firmware>;
using FirmwareDescription = bmcl::Rc<const FirmwareDescriptionObj>;
using FirmwareDescriptions = std::vector<FirmwareDescription>;
using FirmwareCreator = std::function<bmcl::OptionRc<const mccmsg::IFirmware>(const mccmsg::ProtocolValue& id, bmcl::Bytes, const PropertyDescriptionPtrs&, const PropertyDescriptionPtrs&)>;
using StorageCreator = std::function<bmcl::OptionRc<mccmsg::ITmStorage>(const bmcl::OptionRc<const mccmsg::ITmView>& view)>;
}
