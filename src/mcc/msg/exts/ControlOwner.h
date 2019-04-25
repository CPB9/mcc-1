#pragma once
#include "mcc/Config.h"
#include "mcc/msg/exts/ITmExtension.h"
#include <bmcl/Option.h>


namespace mccmsg {

enum class ControlOwner : uint8_t { undefined, me, other };

class MCC_MSG_DECLSPEC TmControlOwner : public ITmSimpleExtension
{
public:
    TmControlOwner(const TmExtensionCounterPtr&);
    ~TmControlOwner() override;
    static const TmExtension& id();
    static const char* info();
    const bmcl::Option<ControlOwner>& owner() const;
    void set(bmcl::SystemTime t, const bmcl::Option<ControlOwner>& speed);
private:
    bmcl::Option<ControlOwner> _owner;
};

}
