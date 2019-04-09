#include "mcc/msg/exts/UavState.h"
#include <cmath>

namespace mccmsg {

TmUavState::TmUavState(const TmExtensionCounterPtr& counter) : ITmSimpleExtension(id(), info(), counter) {}
TmUavState::~TmUavState() {}
const TmExtension& TmUavState::id() { static auto i = TmExtension::createOrNil("{6ad03a5c-afef-4dd1-a598-2e61936ab29b}"); return i; }
const char* TmUavState::info() { return "uav state"; }
const bmcl::Option<bool>& TmUavState::armed() const { return _armed; }
const bmcl::Option<uint8_t>& TmUavState::battery() const { return _battery; }
void TmUavState::set(bmcl::SystemTime t, bool armed, uint8_t battery)
{
    bool changed = ((_armed != armed) || (_battery != battery));
    _armed = armed;
    _battery = battery;
    updated_(t, changed);
}

}
