#pragma once

#include "mcc/crashdump/Config.h"

namespace mcccrashdump {

MCC_CRASHDUMP_DECLSPEC void installProcessCrashHandlers();

MCC_CRASHDUMP_DECLSPEC void installThreadCrashHandlers();

}
