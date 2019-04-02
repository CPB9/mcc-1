#include "mcc/msg/exts/NamedAccess.h"

namespace mccmsg {

INamedAccess::INamedAccess(const TmExtensionCounterPtr& counter) : ITmExtension(id(), info(), counter) {}
INamedAccess::~INamedAccess() {}
const TmExtension& INamedAccess::id() { static auto i = TmExtension::createOrNil("{a5a8e043-6822-44f8-9eb5-281c82feef9c}"); return i; }
const char* INamedAccess::info() { return "named access for tm params"; }

}
