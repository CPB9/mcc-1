#include "mcc/ui/Dialog.h"

namespace mccui {

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    setModal(true);
    setWindowFlags(windowFlags() & (~Qt::WindowContextHelpButtonHint));
}
}
