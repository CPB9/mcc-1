#pragma once

#include "mcc/hm/Config.h"
#include "mcc/hm/Gl1File.h"
#include "mcc/hm/Altitude.h"
#include "mcc/hm/HmReader.h"
#include "mcc/geo/Point.h"

#include <bmcl/Fwd.h>
#include <bmcl/OptionRc.h>

#include <QString>

#include <string>
#include <vector>
#include <cstdint>
#include <cmath>

namespace mcchm {

class Gl1File;
class Gl30AllFile;
class SrtmFileCache;

class MCC_HM_DECLSPEC SrtmReader : public HmReader {
public:
    SrtmReader(const RcGeod* wgs84Geod, const SrtmFileCache* cache);
    SrtmReader(const RcGeod* wgs84Geod, const QString& rootPath);
    ~SrtmReader() override;

    Altitude readAltitude(mccgeo::LatLon latLon, double precisionArcSecond = 0) const override;
    const SrtmReader* clone() const override;

    Altitude readGl30Altitude(mccgeo::LatLon latLon) const;
    Altitude readGl1Altitude(mccgeo::LatLon latLon) const;

    const SrtmFileCache* cache() const;
    void setCache(const SrtmFileCache* cache);

private:
    Rc<const SrtmFileCache> _cache;
    bmcl::OptionRc<const Gl30AllFile> _gl30AllFile;
    mutable Gl1FileDesc _file;
};
}
