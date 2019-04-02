#include "mcc/ui/UiPlugin.h"

namespace mccui {

UiPlugin::UiPlugin(const char* id)
    : mccplugin::Plugin(id)
{
}

UiPlugin::~UiPlugin()
{
}

void UiPlugin::setMccMainWindow(QWidget* parent)
{
    (void)parent;
}

Qt::Alignment UiPlugin::alignment() const
{
    return Qt::AlignCenter;
}
}
