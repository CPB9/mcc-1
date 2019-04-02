#include "mcc/ui/SettingsPage.h"
#include <QIcon>

namespace mccui {

SettingsPage::SettingsPage(QWidget* parent)
    : QWidget(parent)
{
}

SettingsPage::~SettingsPage()
{
}

QIcon SettingsPage::pageIcon() const
{
    return QIcon();
}
}
