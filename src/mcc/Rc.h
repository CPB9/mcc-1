#pragma once

#include "mcc/Config.h"

#include <bmcl/Rc.h>
#include <bmcl/ThreadSafeRefCountable.h>

#include <cstddef>

namespace mcc {

template <typename T>
using Rc = bmcl::Rc<T>;

using RefCountable = bmcl::ThreadSafeRefCountable<std::size_t>;
}
