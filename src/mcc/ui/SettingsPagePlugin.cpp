#include "mcc/ui/SettingsPagePlugin.h"
#include "mcc/ui/SettingsPage.h"

namespace mccui {

SettingsPagePlugin::SettingsPagePlugin()
    : UiPlugin(SettingsPagePlugin::id)
{
}

SettingsPagePlugin::SettingsPagePlugin(mccui::SettingsPage* p)
    : UiPlugin(SettingsPagePlugin::id)
    , _settingsPage(p)
{
}

SettingsPagePlugin::~SettingsPagePlugin()
{
}

std::unique_ptr<mccui::SettingsPage> SettingsPagePlugin::takeSettingsPage()
{
    return std::move(_settingsPage);
}

void SettingsPagePlugin::setSettingsPage(mccui::SettingsPage* page)
{
    _settingsPage.reset(page);
}

bool SettingsPagePlugin::hasSettingsPage() const
{
    return _settingsPage != nullptr;
}
}
