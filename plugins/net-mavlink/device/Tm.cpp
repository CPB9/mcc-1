#include "device/Tm.h"
#include "device/Px4Fsm.h"
#include "mcc/msg/SubHolder.h"

namespace mccmav {

TmView::TmView(const mccmsg::Device& device, const bmcl::OptionRc<const Firmware>& f)
    : mccmsg::ITmView(device), _firmware(f)
{
}

TmView::~TmView()
{
}

const bmcl::OptionRc<const Firmware>& TmView::firmware() const
{
    return _firmware;
}

ITmUpdateVisitor::ITmUpdateVisitor() {}
ITmUpdateVisitor::~ITmUpdateVisitor() {}

TmUpdateMavlink::TmUpdateMavlink(const mccmsg::Device& device) : ITmViewUpdate(device) {}
TmUpdateMavlink::~TmUpdateMavlink() {}

TmViewUpdate::TmViewUpdate(const mccmsg::Device& device)
    : mccmsg::ITmViewUpdate(device)
{
}

TmViewUpdate::~TmViewUpdate()
{
}

TmStorage::TmStorage(const bmcl::OptionRc<const TmView>& v)
    : mccmsg::ITmStorage()
    , _state(new StateStorage(counter()))
    , _names(new NamedAccess(counter()))
{
    if (v.isNone())
        return;
    addExtension(_state);
    addExtension(_names);
    set(v.unwrap().get());
}

TmStorage::~TmStorage() { }
const bmcl::OptionRc<const Firmware>& TmStorage::firmware() const { return _firmware; }

void TmStorage::set(const mccmsg::ITmView* v)
{
    auto view = static_cast<const TmView*>(v);
    if (_firmware == view->firmware())
        return;
    _firmware = view->firmware();
    _state->set(_firmware);
    removeAllHandlers();
    _values.clear();
    _values.resize(_firmware->paramsDescription().size());
}

void TmStorage::update(const mccmsg::ITmViewUpdate* u)
{
    auto update = static_cast<const TmUpdateMavlink*>(u);
    update->visit(*this);
}

void TmStorage::visit(const TmUpdateParam* param)
{
    if (_values.empty())
        return;

    if (param->value().index > _values.size() || param->value().index < 0)
    {
        assert(false);
        return;
    }

    _values[param->value().index] = param->value();
    _names->set(param->value());
}

void TmStorage::visit(const TmUpdateMode* mode)
{
    _state->set(mode->time(), mode->baseMode(), mode->customMode(), mode->systemState());
}

void TmStorage::visit(const TmUpdateMsg* msg)
{

}

bmcl::Option<mccmav::ParamValue> TmStorage::valueByName(const std::string& name) const
{
    auto it = std::find_if(_values.begin(), _values.end(), [&name](const ParamValue& pv) { return pv.name == name; });
    if (it == _values.end())
        return bmcl::None;
    return *it;
}

NamedAccess& TmStorage::namedAccess()
{
    return *_names;
}

TmUpdateParam::TmUpdateParam(const mccmsg::Device& device, const ParamValue& value)
    : TmUpdateMavlink(device), _value(value)
{

}

TmUpdateParam::~TmUpdateParam()
{

}

void TmUpdateParam::visit(ITmUpdateVisitor& visitor) const
{
    visitor.visit(this);
}


const ParamValue& TmUpdateParam::value() const
{
    return _value;
}

StateStorage::StateStorage(const mccmsg::TmExtensionCounterPtr& counter)
    : mccmsg::IStateStorage(counter), _baseMode(-1), _customMode(-1), _systemStatus(-1)
{

}

StateStorage::~StateStorage()
{

}

bmcl::Option<int> StateStorage::state() const
{
    return _state;
}

bmcl::Option<int> StateStorage::subState() const
{
    return _subState;
}

const QString& StateStorage::stateStr() const
{
    return _stateStr;
}

const QString& StateStorage::subStateStr() const
{
    return _subStateStr;
}

void StateStorage::set(const bmcl::OptionRc<const Firmware>& firmware)
{
    _firmware = firmware;
}

void StateStorage::set(const bmcl::SystemTime& t, uint8_t baseMode, uint32_t customMode, uint8_t systemState)
{
    if (_firmware.isNone() || !_firmware->isPx4())
        return;

    if (_baseMode == baseMode && _customMode == customMode && _systemStatus == systemState)
    {
        updated_(t, false);
        return;
    }

    _stateStr.clear();
    _subStateStr.clear();
    DeviceState state(baseMode, customMode, systemState);
    _state = baseMode;
    _subState = customMode;

    _stateStr = state.toString();
    if (state.isEmergency())
        _subStateStr = "EMERGENCY";
    else if (state.isArmed())
        _subStateStr = "ARMED";
    else
        _subStateStr = "DISARMED";

    updated_(t, true);
}

TmUpdateMode::TmUpdateMode(const mccmsg::Device& device, uint8_t baseMode, uint32_t customMode, uint8_t systemState)
    : TmUpdateMavlink(device), _baseMode(baseMode), _customMode(customMode), _systemState(systemState)
{
}

TmUpdateMode::~TmUpdateMode() { }
void TmUpdateMode::visit(ITmUpdateVisitor& visitor) const { visitor.visit(this); }
uint8_t TmUpdateMode::baseMode() const { return _baseMode; }
uint32_t TmUpdateMode::customMode() const { return _customMode; }
uint8_t TmUpdateMode::systemState() const { return _systemState; }

TmUpdateMsg::TmUpdateMsg(const mccmsg::Device& device, const MavlinkMessagePtr& msg) : TmUpdateMavlink(device), _msg(msg) {}
TmUpdateMsg::~TmUpdateMsg() {}
void TmUpdateMsg::visit(ITmUpdateVisitor& visitor) const { visitor.visit(this); }
const MavlinkMessagePtr& TmUpdateMsg::msg() const { return _msg; }

NamedAccess::NamedAccess(const mccmsg::TmExtensionCounterPtr& counter) : mccmsg::INamedAccess(counter) {}
NamedAccess::~NamedAccess() {}
void NamedAccess::removeAllHandlers() { _handlers.clear(); }
void NamedAccess::removeHandler(const bmcl::Option<mccmsg::HandlerId>& id)
{
    if (id.isNone())
        return;
    const auto i = std::find_if(_handlers.begin(), _handlers.end(), [i = id.unwrap()](const auto& item) { return item.id == i; });
    if (i == _handlers.end())
        return;
    _handlers.erase(i);
}

bmcl::Option<mccmsg::SubHolder> NamedAccess::addHandler(bmcl::StringView name, mccmsg::ValueHandler&& handler, bool onChangeOnly)
{
    auto id = nextCounter();

    auto l = [onChangeOnly, h = std::move(handler), s = name.toStdString()](const ParamValue& p)
    {
        if (p.name != s)
            return;
        assert(!onChangeOnly); //нужно откуда-то взять предыдущее значение для сравнения
        if (!onChangeOnly)
            h(p.value, bmcl::SystemClock::now());
    };

    _handlers.emplace_back(id, std::move(l));
    return mccmsg::SubHolder(id, this);
}

bmcl::Option<mccmsg::SubHolder> NamedAccess::addHandler(bmcl::StringView name, NativeHandler&& handler)
{
    auto id = nextCounter();
    _handlers.emplace_back(id, [h = std::move(handler), s = name.toStdString()](const ParamValue& p) { if (p.name == s) h(p); });
    return mccmsg::SubHolder(id, this);
}

mccmsg::SubHolder NamedAccess::addHandler(NativeHandler&& handler)
{
    auto id = nextCounter();
    _handlers.emplace_back(id, [h = std::move(handler)](const ParamValue& p) { h(p); });
    return mccmsg::SubHolder(id, this);
}

void NamedAccess::set(const ParamValue& v)
{
    for (const auto& i : _handlers)
    {
        i.h(v);
    }
}

NamedAccess::Item::Item(mccmsg::HandlerId i, NativeHandler&& h): id(i), h(std::move(h)){}
NamedAccess::Item::~Item() {}


}