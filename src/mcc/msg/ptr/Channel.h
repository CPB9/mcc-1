#pragma once
#include "mcc/Config.h"
#include "mcc/msg/ptr/Message.h"
#include "mcc/msg/ptr/Defs.h"
#include "mcc/msg/obj/Channel.h"
#include "mcc/msg/Stats.h"

namespace mccmsg
{
MSG_DECLARE_NOT(channel, State, StatChannel);
MSG_STANDART_SET(channel);
MSG_DYNAMIC_SET(channel);
}
