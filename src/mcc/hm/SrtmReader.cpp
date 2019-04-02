#include "mcc/hm/SrtmReader.h"
#include "mcc/hm/Gl1File.h"
#include "mcc/hm/Gl30AllFile.h"
#include "mcc/hm/SrtmFileCache.h"

#include <bmcl/Utils.h>
#include <bmcl/Endian.h>
#include <bmcl/MemReader.h>
#include <bmcl/Utils.h>

#include <string>
#include <cmath>
#include <cfloat>
#include <iostream>
#include <QDebug>

namespace mcchm {

SrtmReader::SrtmReader(const RcGeod* wgs84Geod, const SrtmFileCache* cache)
    : HmReader(wgs84Geod)
    , _cache(cache)
{
    _gl30AllFile = _cache->gl30AllFile();
}

SrtmReader::SrtmReader(const RcGeod* wgs84Geod, const QString& rootPath)
    : SrtmReader(wgs84Geod, new SrtmFileCache(rootPath))
{
}

SrtmReader::~SrtmReader()
{
}

void SrtmReader::setCache(const SrtmFileCache* cache)
{
    _cache = cache;
    _gl30AllFile = cache->gl30AllFile();
}

Altitude SrtmReader::readGl30Altitude(mccgeo::LatLon latLon) const
{
    if (_gl30AllFile.isNone()) {
        return bmcl::None;
    }
    return _gl30AllFile.unwrap()->readSampledHeight(latLon.latitude(), latLon.longitude());
}

Altitude SrtmReader::readGl1Altitude(mccgeo::LatLon latLon) const
{
    int latIndex = std::trunc(latLon.latitude());
    double latFrac;
    if (latLon.latitude() >= 0) {
        latFrac = 1.0 + latIndex - latLon.latitude();
    } else {
        latFrac = latIndex - latLon.latitude();
        latIndex -= 1.0;
    }
    int lonIndex = std::trunc(latLon.longitude());
    double lonFrac;
    if (latLon.longitude() >= 0) {
        lonFrac = latLon.longitude() - lonIndex;
    } else {
        lonFrac = 1.0 - lonIndex + latLon.longitude();
        lonIndex -= 1.0;
    }

    if (_file.latIndex != latIndex || _file.lonIndex != lonIndex) {
        _file = _cache->loadFile(latIndex, lonIndex);
    }

    return _file.file->readSampledHeight(latFrac, lonFrac);
}

Altitude SrtmReader::readAltitude(mccgeo::LatLon latLon, double prec) const
{
    if (prec < 20.0) {
        Altitude alt = readGl1Altitude(latLon);
        if (alt.isSome()) {
            return alt;
        }
        return readGl30Altitude(latLon);
    }
    Altitude alt = readGl30Altitude(latLon);
    if (alt.isSome()) {
        return alt;
    }
    return readGl1Altitude(latLon);
}

const SrtmFileCache* SrtmReader::cache() const
{
    return _cache.get();
}

const SrtmReader* SrtmReader::clone() const
{
    return new SrtmReader(geod(), _cache.get());
}
}
