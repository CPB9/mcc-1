#pragma once

#include "mcc/Config.h"
#include "mcc/ui/UiPlugin.h"

namespace mccui {

class SettingsPage;
class SettingsPagePlugin;

typedef std::shared_ptr<SettingsPagePlugin> SettingsPagePluginPtr;

class MCC_UI_DECLSPEC SettingsPagePlugin : public UiPlugin {
public:
    static constexpr const char* id = "mcc::SettingsPagePlugin";

    SettingsPagePlugin();
    explicit SettingsPagePlugin(mccui::SettingsPage* p);
    ~SettingsPagePlugin();

    bool hasSettingsPage() const;
    std::unique_ptr<mccui::SettingsPage> takeSettingsPage();
    void setSettingsPage(mccui::SettingsPage*);

private:
    std::unique_ptr<mccui::SettingsPage> _settingsPage;
};

}
