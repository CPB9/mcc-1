#include "mcc/ui/DialogPlugin.h"
#include "mcc/ui/Dialog.h"

namespace mccui {

DialogPlugin::DialogPlugin()
    : DialogPlugin(nullptr)
{}

DialogPlugin::DialogPlugin(mccui::Dialog* d)
    : UiPlugin(DialogPlugin::id)
    , _dialog(d)
{}

DialogPlugin::~DialogPlugin()
{}

mccplugin::PluginPtr DialogPlugin::create(mccui::Dialog* d)
{
    return std::make_shared<DialogPlugin>(d);
}

void DialogPlugin::setDialog(Dialog* dialog)
{
    _dialog.reset(dialog);
}

std::unique_ptr<mccui::Dialog> DialogPlugin::takeDialog()
{
    return std::move(_dialog);
}

mccui::Dialog* DialogPlugin::dialog() const
{
    return _dialog.get();
}
}
