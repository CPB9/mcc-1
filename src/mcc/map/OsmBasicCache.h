#pragma once

#include "mcc/Config.h"
#include "mcc/map/OsmCache.h"

namespace mccmap {

class MCC_MAP_DECLSPEC OsmBasicCache : public OsmCache {
public:
    int calcMaxTileZoom() override;
    const char* createServiceName() const override;
    const char* createName() const override;
    QString createDescription() const override;
    const char* createFormat() const override;
    Generator createGenerator() const override;
};
}
