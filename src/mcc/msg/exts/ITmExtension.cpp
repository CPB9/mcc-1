#include "mcc/msg/SubHolder.h"
#include "mcc/msg/exts/ITmExtension.h"
#include <bmcl/Option.h>
#include <bmcl/Rc.h>
#include <algorithm>

namespace mccmsg {

TmExtensionCounter::TmExtensionCounter() : _counter(0) {}
TmExtensionCounter::~TmExtensionCounter() {}
HandlerId TmExtensionCounter::next() { return ++_counter; }


ITmExtension::ITmExtension(const TmExtension& name, const char* info, const TmExtensionCounterPtr& counter) : _name(name), _info(info), _counter(counter) {}
ITmExtension::~ITmExtension() {}
const TmExtension& ITmExtension::name() const { return _name; }
const std::string& ITmExtension::info() const { return _info; }
HandlerId ITmExtension::nextCounter() { return _counter->next(); }

ITmSimpleExtension::ITmSimpleExtension(const TmExtension& name, const char* info, const TmExtensionCounterPtr& counter) : ITmExtension(name, info, counter) {}
ITmSimpleExtension::~ITmSimpleExtension() {}
const bmcl::SystemTime& ITmSimpleExtension::updated() const { return _updated; }
const bmcl::SystemTime& ITmSimpleExtension::changed() const { return _changed; }
void ITmSimpleExtension::updated(bmcl::SystemTime t) { updated_(t, false); }

ITmSimpleExtension::Item::Item(HandlerId i, Handler&& h) : i(i), h(std::move(h)) {}
ITmSimpleExtension::Item::~Item() {}

SubHolder ITmSimpleExtension::addHandler(Handler&& handler, bool onChangeOnly)
{
    auto id = nextCounter();
    if (onChangeOnly)
        _changeHandler.emplace_back(id, std::move(handler));
    else
        _updateHandler.emplace_back(id, std::move(handler));
    return SubHolder(id, this);
}

void ITmSimpleExtension::removeHandler(const bmcl::Option<HandlerId>& id)
{
    if (id.isNone())
        return;
    {
        auto i = std::find_if(_updateHandler.begin(), _updateHandler.end(), [name = id.unwrap()](const auto& i) { return i.i == name; });
        if (i != _updateHandler.end())
            _updateHandler.erase(i);
    }
    {
        auto i = std::find_if(_changeHandler.begin(), _changeHandler.end(), [name = id.unwrap()](const auto& i) { return i.i == name; });
        if (i != _changeHandler.end())
            _changeHandler.erase(i);
    }
}

void ITmSimpleExtension::removeAllHandlers()
{
    _changeHandler.clear();
    _updateHandler.clear();
}

void ITmSimpleExtension::updated_(bmcl::SystemTime t, bool changed)
{
    _updated = t;
    if (changed)
    {
        _changed = t;
        for (const auto& i : _changeHandler)
        {
            i.h();
        }
    }
    else
    {
        for (const auto& i : _updateHandler)
        {
            i.h();
        }
    }
}

}
