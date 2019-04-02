#include "mcc/ui/ColorDialogOptions.h"

#include <QProcessEnvironment>

namespace mccui {

QColorDialog::ColorDialogOptions colorDialogOptions()
{
    if(QProcessEnvironment::systemEnvironment().value("XDG_CURRENT_DESKTOP").contains("unity", Qt::CaseInsensitive)) {
        return QColorDialog::ColorDialogOptions();
    }
    return QColorDialog::ColorDialogOptions() | QColorDialog::DontUseNativeDialog;
}
}
