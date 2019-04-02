#pragma once
#include <string>
#include <caf/atom.hpp>
#include "mcc/net/Cmd.h"
#include "mcc/net/TmHelper.h"

using ::mccnet::TmHelper;

namespace mccmav {

using timer_atom = caf::atom_constant<caf::atom("timer")>;
using activated_atom = caf::atom_constant<caf::atom("act")>;
using deactivated_atom = caf::atom_constant<caf::atom("deact")>;

using frmupdate_atom = caf::atom_constant<caf::atom("frmupdate")>;
using cancel_atom = caf::atom_constant<caf::atom("cancel")>;

using mission_state_atom = caf::atom_constant<caf::atom("mission")>;

using waypoint_msg_atom = caf::atom_constant<caf::atom("wp_msg")>;
using write_param_atom = caf::atom_constant<caf::atom("par_write")>;

using waypoint_msg_no_rep_atom = caf::atom_constant<caf::atom("wp_msg_nr")>;
using send_msg_atom = caf::atom_constant<caf::atom("sendmsg")>;

using params_dump_atom = caf::atom_constant<caf::atom("paramdump")>;
using set_param_atom = caf::atom_constant<caf::atom("setparam")>;

using rbt_cmd_atom = caf::atom_constant<caf::atom("rbtcmd")>;
}