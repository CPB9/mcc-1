#include "mcc/plugin/PluginData.h"

#include <cstring>

namespace mccplugin {

PluginData::PluginData(const char* id)
    : _dataId(id)
{
}

PluginData::~PluginData()
{
}

const char* PluginData::dataId() const
{
    return _dataId;
}

bool PluginData::hasDataId(const char* id) const
{
    return std::strcmp(_dataId, id) == 0;
}
}
