#pragma once
#include "mcc/Config.h"
#include <string>
#include <chrono>
#include <bmcl/Buffer.h>
#include "mcc/msg/Objects.h"
#include "mcc/msg/Property.h"

namespace mccmsg {

class MCC_MSG_DECLSPEC ProtocolId
{
public:
    ProtocolId();
    ProtocolId(const Device& device, const Protocol& protocol, DeviceId id);
    const Protocol& protocol() const;
    DeviceId id() const;
    const Device& device() const;
private:
    DeviceId _id;
    Device   _device;
    Protocol _protocol;
};
using ProtocolIds = std::vector<ProtocolId>;

class MCC_MSG_DECLSPEC ProtocolValue
{
public:
    ProtocolValue();
    ProtocolValue(const Protocol& protocol, const std::string&);
    ProtocolValue(const Protocol& protocol, std::string&&);
    const Protocol& protocol() const;
    const std::string& value() const;
    bool operator==(const ProtocolValue& other) const;
    bool operator!=(const ProtocolValue& other) const;
private:
    std::string _value;
    Protocol    _protocol;
};
using ProtocolIds = std::vector<ProtocolId>;

class MCC_MSG_DECLSPEC ProtocolDescriptionObj : public mcc::RefCountable
{
public:
    ProtocolDescriptionObj(const Protocol& name
                        , bool shareable
                        , bool logging
                        , std::chrono::milliseconds timeout
                        , bmcl::StringView info
                        , bmcl::StringView param_info
                        , bmcl::Buffer&& pixmap
                        , PropertyDescriptionPtrs&& req
                        , PropertyDescriptionPtrs&& opt);
    ~ProtocolDescriptionObj() override;

    const Protocol& name() const;
    const std::chrono::milliseconds& timeout() const;
    const std::string& info() const;
    const std::string& param_info() const;
    const bmcl::Buffer& pixmap() const;
    bool shareable() const;
    bool logging() const;
    DeviceId maxDeviceId() const;
    const PropertyDescriptionPtrs& requiredProperties() const;
    const PropertyDescriptionPtrs& optionalProperties() const;
private:
    Protocol    _name;
    std::chrono::milliseconds _timeout;
    std::string _info;
    std::string _param_info;
    bmcl::Buffer _pixmap;
    bool _shareable;
    bool _logging;
    PropertyDescriptionPtrs _required;
    PropertyDescriptionPtrs _optional;
};

using ProtocolIds = std::vector<ProtocolId>;
using Protocols = std::vector<Protocol>;
using ProtocolDescription = bmcl::Rc<const ProtocolDescriptionObj>;
using ProtocolDescriptions = std::vector<ProtocolDescription>;
}
