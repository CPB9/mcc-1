#pragma once

#include "mcc/Config.h"
#include "mcc/ui/UiPlugin.h"

#include <bmcl/OptionPtr.h>

class QMenu;

namespace mccui {

class MainMenuPlugin;

typedef std::shared_ptr<MainMenuPlugin> MainMenuPluginPtr;

class MCC_UI_DECLSPEC MainMenuPlugin : public UiPlugin {
public:
    static constexpr const char* id = "mcc::MainMenuPlugin";

    MainMenuPlugin();
    explicit MainMenuPlugin(QMenu* menu);
    ~MainMenuPlugin();

    std::unique_ptr<QMenu> takeMenu();
    void setMenu(QMenu* menu);

    virtual void guiPostInit() {}
private:
    std::unique_ptr<QMenu> _menu;
};

}
