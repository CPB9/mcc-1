#include "mcc/map/drawables/WithPosition.h"
#include <QApplication>
#include <QClipboard>

namespace mccmap {

void copyToClipboard(QString&& text)
{
    QApplication::clipboard()->setText(std::move(text));
}

}
