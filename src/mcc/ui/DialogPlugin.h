#pragma once

#include "mcc/Config.h"
#include "mcc/ui/UiPlugin.h"

namespace mccui {

class Dialog;
class DialogPlugin;

typedef std::shared_ptr<DialogPlugin> DialogPluginPtr;

class MCC_UI_DECLSPEC DialogPlugin : public UiPlugin {
public:
    static constexpr const char* id = "mcc::DialogPlugin";

    explicit DialogPlugin();
    explicit DialogPlugin(mccui::Dialog* d);
    ~DialogPlugin() override;

    static mccplugin::PluginPtr create(mccui::Dialog* d);

    void setDialog(mccui::Dialog* dialog);
    std::unique_ptr<mccui::Dialog> takeDialog();
    mccui::Dialog* dialog() const;

private:
    std::unique_ptr<mccui::Dialog> _dialog;
};

}
