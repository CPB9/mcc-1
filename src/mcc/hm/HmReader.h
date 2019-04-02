#pragma once

#include "mcc/hm/Config.h"
#include "mcc/hm/Rc.h"
#include "mcc/hm/Altitude.h"
#include "mcc/geo/LatLon.h"
#include "mcc/geo/Position.h"
#include "mcc/geo/Geod.h"
#include "mcc/geo/Point.h"

#include <bmcl/Fwd.h>

#include <vector>

namespace mcchm {

class RcGeod : public mccgeo::Geod, public RefCountable {
public:
    using mccgeo::Geod::Geod;
};

class MCC_HM_DECLSPEC HmReader : public RefCountable  {
public:
    HmReader(const RcGeod* wgs84Geod);
    ~HmReader() override;

    virtual Altitude readAltitude(mccgeo::LatLon latLon, double precisionArcSecond = 0) const = 0;
    virtual const HmReader* clone() const = 0;

    virtual void readAltitudeMatrix(bmcl::ArrayView<double> lats,
                                    bmcl::ArrayView<double> lons,
                                    double* matrix,
                                    double precisionArcSecond = 0,
                                    double defaultValue = 0) const;

    virtual std::vector<mccgeo::PositionAndDistance> profile(mccgeo::LatLon latLon1,
                                                             mccgeo::LatLon latLon2,
                                                             double step,
                                                             double prec = 0) const;

    virtual mccgeo::PointVector relativePointProfile(mccgeo::LatLon latLon1,
                                                     mccgeo::LatLon latLon2,
                                                     double step,
                                                     double prec = 0) const;

    virtual std::vector<mccgeo::PositionAndDistance> profileAutostep(mccgeo::LatLon latLon1,
                                                                     mccgeo::LatLon latLon2,
                                                                     double prec = 0) const;
    virtual mccgeo::PointVector relativePointProfileAutostep(mccgeo::LatLon latLon1,
                                                             mccgeo::LatLon latLon2,
                                                             double prec = 0) const;

    const RcGeod* geod() const;

private:
    Rc<const RcGeod> _wgs84Geod;
};

class MCC_HM_DECLSPEC EmptyHmReader : public HmReader  {
public:
    using HmReader::HmReader;

    EmptyHmReader();

    Altitude readAltitude(mccgeo::LatLon latLon, double precisionArcSecond = 0) const override;
    const HmReader* clone() const override;
};

}
