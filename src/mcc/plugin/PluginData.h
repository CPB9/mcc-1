#pragma once

#include "mcc/plugin/Config.h"

namespace mccplugin {

class MCC_PLUGIN_DECLSPEC PluginData {
public:
    virtual ~PluginData();

    const char* dataId() const;
    bool hasDataId(const char* id) const;

protected:
    explicit PluginData(const char* id);

private:
    const char* _dataId;
};
}
