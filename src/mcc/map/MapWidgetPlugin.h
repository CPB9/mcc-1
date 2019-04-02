#pragma once

#include "mcc/Config.h"
#include "mcc/plugin/Plugin.h"
#include <functional>

namespace mccmap {
class UserWidget;
}

namespace mccmap {

class MapWidgetPlugin;

typedef std::shared_ptr<MapWidgetPlugin> MapWidgetPluginPtr;

class MCC_MAP_DECLSPEC MapWidgetPlugin : public mccplugin::Plugin {
using MapWidgetCreator  = std::function<mccmap::UserWidget*()>;
public:
    static constexpr const char* id = "mcc::MapWidgetPlugin";

    MapWidgetPlugin(mccmap::UserWidget* widget);
    MapWidgetPlugin();
    ~MapWidgetPlugin();

    std::unique_ptr<mccmap::UserWidget> takeMapWidget();
    void setMapWidget(mccmap::UserWidget* widget);

private:
    std::unique_ptr<mccmap::UserWidget> _widget;
};

}
