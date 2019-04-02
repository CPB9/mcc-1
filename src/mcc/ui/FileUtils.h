#pragma once

#include "mcc/Config.h"

#include <bmcl/Result.h>

class QString;

namespace mccui {

MCC_UI_DECLSPEC bmcl::Result<bool, QString> copyRecursively(const QString& from, const QString& to);

}
