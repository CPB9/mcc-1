#include "mcc/msg/exts/ErrStorage.h"

namespace mccmsg {

IErrStorage::IErrStorage(const TmExtensionCounterPtr& counter) : ITmSimpleExtension(id(), info(), counter) {}
IErrStorage::~IErrStorage() {}
const TmExtension& IErrStorage::id() { static auto i = TmExtension::createOrNil("{950ed5b9-4431-48a9-bc35-15cc1958c51e}"); return i; }
const char* IErrStorage::info() { return "errors"; }
IErr::~IErr() {}

}
