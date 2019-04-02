#pragma once

#include "mcc/Config.h"

class QWidget;
class QString;

namespace mccui {

MCC_UI_DECLSPEC QWidget* findMainWindow();
MCC_UI_DECLSPEC void showInGraphicalShell(const QString& path);

}
