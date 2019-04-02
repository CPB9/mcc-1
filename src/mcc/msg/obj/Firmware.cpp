#include <bmcl/StringView.h>
#include "mcc/msg/obj/Firmware.h"

namespace mccmsg {

IFirmware::IFirmware(const ProtocolValue& id, const PropertyDescriptionPtrs& req, const PropertyDescriptionPtrs& opt) : _id(id), _required(req), _optional(opt){}
IFirmware::~IFirmware() {}
const PropertyDescriptionPtrs& IFirmware::required() const { return _required; }
const PropertyDescriptionPtrs& IFirmware::optional() const { return _optional; }
const ProtocolValue& IFirmware::id() const { return _id; }

FirmwareDescriptionObj::FirmwareDescriptionObj(const Firmware& name, const ProtocolValue& id, const IFirmwarePtr& frm) : _name(name), _id(id), _frm(frm) {}
FirmwareDescriptionObj::~FirmwareDescriptionObj() {}

const Firmware& FirmwareDescriptionObj::name() const { return _name; }
const ProtocolValue& FirmwareDescriptionObj::id() const { return _id; }
const IFirmwarePtr& FirmwareDescriptionObj::frm() const { return _frm; }

}
