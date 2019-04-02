#pragma once
#include "mcc/Config.h"
#include "mcc/msg/exts/ITmExtension.h"
#include <bmcl/Logging.h>
#include <bmcl/OptionPtr.h>
#include <set>

class QString;

namespace mccmsg {

using ErrorMsgId = uint32_t;
using ErrorMsgIds = std::set<ErrorMsgId>;

class MCC_MSG_DECLSPEC IErr
{
public:
    virtual ~IErr();
    virtual bmcl::LogLevel level() const = 0;
    virtual ErrorMsgId id() const = 0;
    virtual bool isHidden() const = 0;
    virtual QString qtext() const = 0;
};

class MCC_MSG_DECLSPEC IErrStorage : public ITmSimpleExtension
{
public:
    IErrStorage(const TmExtensionCounterPtr&);
    ~IErrStorage() override;
    static const TmExtension& id();
    static const char* info();
    virtual const ErrorMsgIds& visible() const = 0;
    virtual const ErrorMsgIds& all() const = 0;
    virtual const ErrorMsgIds& hidden() const = 0;
    virtual bmcl::OptionPtr<const IErr> get(ErrorMsgId) const = 0;
    virtual void hide(ErrorMsgId, bool) = 0;
};

}
