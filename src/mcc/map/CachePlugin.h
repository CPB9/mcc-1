#pragma once

#include "mcc/Config.h"

#include "mcc/map/Fwd.h"
#include "mcc/plugin/Plugin.h"
#include "mcc/map/OnlineCache.h"

namespace mccmap {

class CachePlugin;

typedef std::shared_ptr<CachePlugin> CachePluginPtr;

class MCC_MAP_DECLSPEC CachePlugin : public mccplugin::Plugin {
public:
    static constexpr const char* id = "mccmap::OnlineCache";

    CachePlugin(OnlineCache* cache);
    ~CachePlugin();

    template <typename T>
    static CachePluginPtr create()
    {
        return std::make_shared<CachePlugin>(OnlineCache::create<T>().get());
    }

    OnlineCache* cache() const;
    void setCache(OnlineCache* cache);

private:
    Rc<OnlineCache> _cache;
};

}
