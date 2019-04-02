#include "mcc/plugin/Plugin.h"

#include <cstring>

namespace mccplugin {

Plugin::Plugin(const char* typeId)
    : _typeId(typeId)
{
}

Plugin::~Plugin()
{
}

bool Plugin::init(PluginCache* cache)
{
    (void)cache;
    return true;
}

const char* Plugin::typeId() const
{
    return _typeId;
}

bool Plugin::hasTypeId(const char* id) const
{
    return std::strcmp(_typeId, id) == 0;
}

void Plugin::postInit(PluginCache* cache)
{
    (void)cache;
}

int64_t Plugin::priority() const
{
    return 0;
}
}
