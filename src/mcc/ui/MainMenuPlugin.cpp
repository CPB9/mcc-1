#include "mcc/ui/MainMenuPlugin.h"

#include <QMenu>

namespace mccui {

MainMenuPlugin::MainMenuPlugin()
    : UiPlugin(MainMenuPlugin::id)
{
}

MainMenuPlugin::MainMenuPlugin(QMenu* menu)
    : UiPlugin(MainMenuPlugin::id)
    , _menu(menu)
{
}

MainMenuPlugin::~MainMenuPlugin()
{
}

std::unique_ptr<QMenu> MainMenuPlugin::takeMenu()
{
    return std::move(_menu);
}

void MainMenuPlugin::setMenu(QMenu* menu)
{
    _menu.reset(menu);
}

}
