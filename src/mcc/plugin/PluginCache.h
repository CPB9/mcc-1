#pragma once

#include "mcc/plugin/Config.h"
#include "mcc/plugin/Rc.h"
#include "mcc/plugin/Plugin.h"

#include <bmcl/OptionPtr.h>

#include <vector>
#include <map>

class QString;

namespace mccplugin {

class Plugin;
class PluginData;

class MCC_PLUGIN_DECLSPEC PluginCache : public RefCountable {
public:
    PluginCache();
    ~PluginCache();

    inline const std::vector<PluginPtr>& plugins() const;

    template <typename T>
    bmcl::OptionPtr<T> findPluginData()
    {
        PluginData* d = findData(T::id);
        T* data = dynamic_cast<T*>(d);
        return bmcl::OptionPtr<T>(data);
    }

    bool addPluginData(std::unique_ptr<PluginData>&& data);

protected:
    PluginData* findData(const char* name);

    std::vector<PluginPtr> _plugins;
    std::map<std::string, std::unique_ptr<PluginData>> _pluginData;
};

inline const std::vector<PluginPtr>& PluginCache::plugins() const
{
    return _plugins;
}

class MCC_PLUGIN_DECLSPEC PluginCacheWriter : public PluginCache {
public:
    PluginCacheWriter();
    ~PluginCacheWriter();

    void loadAllFromDir(const QString& path);
    bool loadLibrary(const QString& path);

    void addPlugin(const PluginPtr& plugin);
    void addPlugin(PluginPtr&& plugin);

    void initPlugins();

    void clear();

private:
    void sortByPriority();
};
}
