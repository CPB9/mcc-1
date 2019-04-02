#include "mcc/map/OsmBasicCache.h"
#include "mcc/map/LinkGenerator.h"

namespace mccmap {

int OsmBasicCache::calcMaxTileZoom()
{
    return 18;
}

QString OsmBasicCache::createDescription() const
{
    return "";
}

const char* OsmBasicCache::createFormat() const
{
    return "png";
}

OnlineCache::Generator OsmBasicCache::createGenerator() const
{
    return OsmGen("", ".tile.openstreetmap.org", 'a', 3);
}

const char* OsmBasicCache::createName() const
{
    return "Basic";
}

const char* OsmBasicCache::createServiceName() const
{
    return "osmmapMapnik";
}
}
