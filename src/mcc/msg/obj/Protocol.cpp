#include <bmcl/StringView.h>
#include "mcc/msg/obj/Protocol.h"

namespace mccmsg {

ProtocolId::ProtocolId() :_id(DeviceId(-1)) {}
ProtocolId::ProtocolId(const Device& device, const Protocol& protocol, DeviceId id) : _id(id), _device(device), _protocol(protocol) {}
const Protocol& ProtocolId::protocol() const { return _protocol; }
DeviceId ProtocolId::id() const { return _id; }
const Device& ProtocolId::device() const { return _device; }
//std::string ProtocolId::to_string(const ProtocolId& id) { return id._device.toStdString() + id._protocol.toStdString() + std::to_string(id._id); }

ProtocolValue::ProtocolValue() {}
ProtocolValue::ProtocolValue(const Protocol& protocol, const std::string& value) : _protocol(protocol), _value(value) {}
ProtocolValue::ProtocolValue(const Protocol& protocol, std::string&& value) : _protocol(protocol), _value(std::move(value)) {}
const Protocol& ProtocolValue::protocol() const { return _protocol; }
const std::string& ProtocolValue::value() const { return _value; }
bool ProtocolValue::operator==(const ProtocolValue& other) const { return (_protocol == other.protocol()) && (_value == other.value()); }
bool ProtocolValue::operator!=(const ProtocolValue& other) const { return (_protocol != other.protocol()) || (_value != other.value()); }


ProtocolDescriptionObj::ProtocolDescriptionObj(const Protocol& name
    , bool shareable
    , bool logging
    , std::chrono::milliseconds timeout
    , bmcl::StringView info
    , bmcl::StringView param_info
    , bmcl::Buffer&& pixmap
    , PropertyDescriptionPtrs&& req
    , PropertyDescriptionPtrs&& opt)
    : _name(name)
    , _timeout(timeout)
    , _info(info.toStdString())
    , _param_info(param_info.toStdString())
    , _pixmap(std::move(pixmap))
    , _shareable(shareable)
    , _logging(logging)
    , _required(std::move(req))
    , _optional(std::move(opt))
{
}
ProtocolDescriptionObj::~ProtocolDescriptionObj() {}

const Protocol& ProtocolDescriptionObj::name() const { return _name; }
const std::chrono::milliseconds& ProtocolDescriptionObj::timeout() const { return _timeout; }
const std::string& ProtocolDescriptionObj::info() const { return _info; }
const std::string& ProtocolDescriptionObj::param_info() const { return _param_info; }
const bmcl::Buffer& ProtocolDescriptionObj::pixmap() const { return _pixmap; }
bool ProtocolDescriptionObj::shareable() const { return _shareable; }
bool ProtocolDescriptionObj::logging() const { return _logging; }
DeviceId ProtocolDescriptionObj::maxDeviceId() const { return std::numeric_limits<uint16_t>::max(); }
const PropertyDescriptionPtrs& ProtocolDescriptionObj::requiredProperties() const { return _required; }
const PropertyDescriptionPtrs& ProtocolDescriptionObj::optionalProperties() const { return _optional; }

}
