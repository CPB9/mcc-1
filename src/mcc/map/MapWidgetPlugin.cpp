#include "mcc/map/MapWidgetPlugin.h"
#include "mcc/map/UserWidget.h"

namespace mccmap {

MapWidgetPlugin::MapWidgetPlugin()
    : Plugin(MapWidgetPlugin::id)
{
}

MapWidgetPlugin::MapWidgetPlugin(mccmap::UserWidget* widget)
    : Plugin(MapWidgetPlugin::id)
    , _widget(widget)
{
}

MapWidgetPlugin::~MapWidgetPlugin()
{
}

std::unique_ptr<mccmap::UserWidget> MapWidgetPlugin::takeMapWidget()
{
    return std::move(_widget);
}

void MapWidgetPlugin::setMapWidget(mccmap::UserWidget* widget)
{
    _widget.reset(widget);
}
}
