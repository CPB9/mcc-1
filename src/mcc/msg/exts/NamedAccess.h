#pragma once
#include "mcc/Config.h"
#include "mcc/msg/exts/ITmExtension.h"
#include <bmcl/Option.h>
#include <functional>

namespace mccmsg {

class NetVariant;
using ValueHandler = std::function<void(const NetVariant&, const bmcl::SystemTime&)>;

class MCC_MSG_DECLSPEC INamedAccess : public ITmExtension
{
public:
    INamedAccess(const TmExtensionCounterPtr&);
    ~INamedAccess() override;
    static const TmExtension& id();
    static const char* info();
    virtual bmcl::Option<HandlerId> addHandler(bmcl::StringView name, ValueHandler&& handler) = 0;
};

}
