#include "mcc/map/LayerPlugin.h"
#include "mcc/map/Layer.h"

namespace mccmap {

LayerPlugin::LayerPlugin()
    : mccplugin::Plugin(id)
{
}

LayerPlugin::LayerPlugin(Layer* layer)
    : mccplugin::Plugin(id)
    , _layer(layer)
{
}

LayerPlugin::~LayerPlugin()
{
}

void LayerPlugin::setLayer(Layer* layer)
{
    _layer.reset(layer);
}

bmcl::OptionPtr<Layer> LayerPlugin::layer()
{
    return _layer.get();
}

bmcl::OptionPtr<const Layer> LayerPlugin::layer() const
{
    return _layer.get();
}
}
