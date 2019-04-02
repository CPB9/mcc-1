#include "mcc/map/CachePlugin.h"

namespace mccmap {

CachePlugin::CachePlugin(OnlineCache* cache)
    : mccplugin::Plugin(CachePlugin::id)
    , _cache(cache)
{
}

CachePlugin::~CachePlugin()
{
}

OnlineCache* CachePlugin::cache() const
{
    return _cache.get();
}

void CachePlugin::setCache(OnlineCache* cache)
{
    _cache = cache;
}
}
