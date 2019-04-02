#pragma once
#include "mcc/Config.h"
#include <cstdint>
#include <bmcl/Option.h>


namespace mccnet {

MCC_PLUGIN_NET_DECLSPEC bmcl::Option<uint32_t> netName();

}
