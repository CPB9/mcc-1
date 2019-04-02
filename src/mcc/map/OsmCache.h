#pragma once

#include "mcc/Config.h"
#include "mcc/map/OnlineCache.h"
#include "mcc/geo/MercatorProjection.h"

namespace mccmap {

class MCC_MAP_DECLSPEC OsmCache : public OnlineCache {
public:
    const char* createProvider() const override
    {
        return "OpenStreetMap";
    }

    mccgeo::MercatorProjection::ProjectionType createProjection() const override
    {
        return mccgeo::MercatorProjection::SphericalMercator;
    }
};
}
