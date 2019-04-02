#pragma once

#include <bmcl/Rc.h>
#include <bmcl/ThreadSafeRefCountable.h>

#include <cstddef>

namespace mccplugin {

template <typename T>
using Rc = bmcl::Rc<T>;

using RefCountable = bmcl::ThreadSafeRefCountable<std::size_t>;
}
