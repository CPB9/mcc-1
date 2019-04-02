#pragma once
#include "mcc/Config.h"
#include "mcc/msg/exts/ITmExtension.h"
#include <bmcl/Option.h>


namespace mccmsg {

class MCC_MSG_DECLSPEC TmUavState : public ITmSimpleExtension
{
public:
    TmUavState(const TmExtensionCounterPtr&);
    ~TmUavState() override;
    static const TmExtension& id();
    static const char* info();
    bmcl::Option<bool> armed() const;
    bmcl::Option<uint8_t> battery() const;
    void set(bmcl::SystemTime t);
private:
    bmcl::Option<bool> _armed;
    bmcl::Option<uint8_t> _battery;
};

}
