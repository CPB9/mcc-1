#pragma once

#include "mcc/Config.h"
#include <QColor>

class QPixmap;

namespace mccui {

class MCC_UI_DECLSPEC GraphicsEffectCreator
{
public:
    static QPixmap applyColorEffect(const QPixmap& src, QColor color, float strength = 1.0, float extent = 0.0f);
};
}
