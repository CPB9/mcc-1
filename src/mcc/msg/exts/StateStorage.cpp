#include "mcc/msg/exts/StateStorage.h"

namespace mccmsg {

IStateStorage::IStateStorage(const TmExtensionCounterPtr& counter) : ITmSimpleExtension(id(), info(), counter) {}
IStateStorage::~IStateStorage() {}
const TmExtension& IStateStorage::id() { static auto i = TmExtension::createOrNil("{44c9f59e-7615-4db0-9df9-204d8a89efa0}"); return i; }
const char* IStateStorage::info() { return "uav states"; }

}
