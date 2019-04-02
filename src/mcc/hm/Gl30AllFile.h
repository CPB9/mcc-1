#pragma once

#include "mcc/hm/Config.h"
#include "mcc/hm/Rc.h"
#include "mcc/hm/Altitude.h"
#include "mcc/geo/LatLon.h"

#include <bmcl/Fwd.h>

#include <QFile>

class QString;

namespace mcchm {

class MCC_HM_DECLSPEC Gl30AllFile : public RefCountable {
public:
    ~Gl30AllFile();

    static bmcl::OptionRc<const Gl30AllFile> load(const QString& dirPath);

    SrtmAltitude readHeight(double lat, double lon) const;
    Altitude readSampledHeight(double lat, double lon) const;

    SrtmAltitude readHeight(mccgeo::LatLon latLon) const;
    Altitude readSampledHeight(mccgeo::LatLon latLon) const;

private:
    Gl30AllFile() = default;

    QFile _file;
    const uint16_t* _data;
};

inline SrtmAltitude Gl30AllFile::readHeight(mccgeo::LatLon latLon) const
{
    return readHeight(latLon.latitude(), latLon.longitude());
}

inline Altitude Gl30AllFile::readSampledHeight(mccgeo::LatLon latLon) const
{
    return readSampledHeight(latLon.latitude(), latLon.longitude());
}
}
