#pragma once

#include "mcc/Config.h"

#include <QString>

namespace mccui {

MCC_UI_DECLSPEC QString shortTextLine(const QString& text, int maxSize = 12);
MCC_UI_DECLSPEC QString shortTextLineInMiddle(const QString& text, int maxSize = 12);
MCC_UI_DECLSPEC QString bytesToString(uint64_t bytes, bool withSpace = true);
}
