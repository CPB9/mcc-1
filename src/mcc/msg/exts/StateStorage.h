#pragma once
#include "mcc/Config.h"
#include "mcc/msg/exts/ITmExtension.h"
#include <bmcl/Option.h>
#include <functional>

class QString;

namespace mccmsg {

using StateHandler = std::function<void()>;
class MCC_MSG_DECLSPEC IStateStorage : public ITmSimpleExtension
{
public:
    IStateStorage(const TmExtensionCounterPtr&);
    ~IStateStorage() override;
    static const TmExtension& id();
    static const char* info();
    virtual bmcl::Option<int> state() const = 0;
    virtual bmcl::Option<int> subState() const = 0;
    virtual const QString& stateStr() const = 0;
    virtual const QString& subStateStr() const = 0;
};

}
