#pragma once

#include "mcc/Config.h"
#include "mcc/plugin/Plugin.h"
#include "mcc/map/Rc.h"

#include <bmcl/OptionPtr.h>

namespace mccmap {

class Layer;

class MCC_MAP_DECLSPEC LayerPlugin : public mccplugin::Plugin {
public:
    static constexpr const char* id = "mccmap::Layer";

    LayerPlugin(Layer* layer);
    LayerPlugin();
    ~LayerPlugin();

    bmcl::OptionPtr<Layer> layer();
    bmcl::OptionPtr<const Layer> layer() const;

    void setLayer(Layer* layer);

private:
    Rc<Layer> _layer;
};

}

