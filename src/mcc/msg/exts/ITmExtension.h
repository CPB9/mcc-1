#pragma once
#include "mcc/Config.h"
#include "mcc/Rc.h"
#include "mcc/msg/Objects.h"
#include <bmcl/Rc.h>
#include <bmcl/TimeUtils.h>
#include <functional>
#include <vector>

namespace mccmsg {

using HandlerId = std::size_t;
using Handler = std::function<void()>;

class MCC_MSG_DECLSPEC TmExtensionCounter : public mcc::RefCountable
{
public:
    TmExtensionCounter();
    ~TmExtensionCounter() override;
    HandlerId next();
private:
    HandlerId _counter;
};
using TmExtensionCounterPtr = bmcl::Rc<TmExtensionCounter>;

class MCC_MSG_DECLSPEC ITmExtension : public mcc::RefCountable
{
public:
    ITmExtension(const TmExtension& name, const char* info, const TmExtensionCounterPtr&);
    ~ITmExtension() override;
    const TmExtension& name() const;
    const std::string& info() const;
    virtual void removeHandler(const bmcl::Option<HandlerId>&) = 0;
    virtual void removeAllHandlers() = 0;
protected:
    HandlerId nextCounter();

private:
    TmExtension _name;
    std::string _info;
    TmExtensionCounterPtr _counter;
};
using ITmExtensionPtr = bmcl::Rc<ITmExtension>;

class SubHolder;
class MCC_MSG_DECLSPEC ITmSimpleExtension : public ITmExtension
{
public:
    ITmSimpleExtension(const TmExtension& name, const char* info, const TmExtensionCounterPtr&);
    ~ITmSimpleExtension() override;
    void removeHandler(const bmcl::Option<HandlerId>&) override;
    void removeAllHandlers() override;

    const bmcl::SystemTime& updated() const;
    const bmcl::SystemTime& changed() const;
    void updated(bmcl::SystemTime);
    SubHolder addHandler(Handler&&, bool onChangeOnly);
protected:
    void updated_(bmcl::SystemTime, bool changed);

private:
    bmcl::SystemTime _updated;
    bmcl::SystemTime _changed;
    struct Item
    {
        Item(HandlerId i, Handler&& h);
        ~Item();
        Handler h;
        HandlerId i;
    };
    using Items = std::vector<Item>;
    Items _updateHandler;
    Items _changeHandler;
};



}
