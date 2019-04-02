#pragma once
#include <string>
#include <caf/atom.hpp>
#include "mcc/net/Cmd.h"
#include "mcc/net/TmHelper.h"

using ::mccnet::TmHelper;

namespace mccphoton {

using timer_atom = caf::atom_constant<caf::atom("timer")>;
using activated_atom = caf::atom_constant<caf::atom("act")>;
using deactivated_atom = caf::atom_constant<caf::atom("deact")>;
using rc_channels_atom = caf::atom_constant<caf::atom("rcchnls")>;

}