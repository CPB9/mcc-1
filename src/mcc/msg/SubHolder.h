#pragma once
#include "mcc/Config.h"
#include "mcc/msg/Fwd.h"
#include <bmcl/Option.h>
#include <bmcl/Rc.h>

namespace mccmsg {

class ITmExtension;
using ITmExtensionPtr = bmcl::Rc<ITmExtension>;

class MCC_MSG_DECLSPEC SubHolder
{
public:
    SubHolder();
    SubHolder(HandlerId, ITmExtension*);
    SubHolder(const SubHolder&) = delete;
    SubHolder(SubHolder&&);
    SubHolder& operator=(SubHolder&&);
    ~SubHolder();
    bmcl::Option<HandlerId> takeId();
    void unsub();
private:
    bmcl::Option<HandlerId> _id;
    ITmExtensionPtr _ext;
};

}
