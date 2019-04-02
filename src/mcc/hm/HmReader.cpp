#include "mcc/hm/HmReader.h"
#include "mcc/geo/Constants.h"

#include <bmcl/ArrayView.h>

namespace mcchm {

HmReader::HmReader(const RcGeod* wgs84Geod)
    : _wgs84Geod(wgs84Geod)
{
}

HmReader::~HmReader()
{
}

std::vector<mccgeo::PositionAndDistance> HmReader::profileAutostep(mccgeo::LatLon latLon1, mccgeo::LatLon latLon2, double prec) const
{
    //TODO: implement autostep
    return profile(latLon1, latLon2, 30, prec);
};

mccgeo::PointVector HmReader::relativePointProfileAutostep(mccgeo::LatLon latLon1, mccgeo::LatLon latLon2, double prec) const
{
    //TODO: implement autostep
    return relativePointProfile(latLon1, latLon2, 30, prec);
}

constexpr mccgeo::GeodMask lineMask = mccgeo::GeodMask::Latitude | mccgeo::GeodMask::Longitude | mccgeo::GeodMask::DistanceIn;

std::vector<mccgeo::PositionAndDistance> HmReader::profile(mccgeo::LatLon latLon1, mccgeo::LatLon latLon2, double step, double prec) const
{
    std::vector<mccgeo::PositionAndDistance> profile;

    double d = 0;
    double a1 = 0;
    double currentD = 0;

    _wgs84Geod->inverse(latLon1, latLon2, &d, &a1, 0);

    if (!std::isnormal(d)) {
        profile.emplace_back();
        return profile;
    }

    if (!std::isnormal(d) || !std::isnormal(step) || !std::isnormal(d / step)) {
        return profile;
    }
    profile.reserve(std::ceil(d / step) + 1);

    mccgeo::GeodLine l(*_wgs84Geod.get(), latLon1, a1, lineMask);
    mccgeo::LatLon curLatLon;
    while (d > currentD) {
        l.position(currentD, &curLatLon, 0);
        auto alt = readAltitude(curLatLon, prec).unwrapOr(0);
        profile.emplace_back(curLatLon, alt, currentD);
        currentD += step;
    }
    auto alt = readAltitude(latLon2, prec).unwrapOr(0);
    profile.emplace_back(latLon2, alt, d);
    return profile;
};

mccgeo::PointVector HmReader::relativePointProfile(mccgeo::LatLon latLon1, mccgeo::LatLon latLon2, double step, double prec) const
{
    double d = 0;
    double a1 = 0;
    double currentD = 0;
    mccgeo::PointVector profile;
    _wgs84Geod->inverse(latLon1, latLon2, &d, &a1, 0);

    if (!std::isnormal(d)) {
        profile.emplace_back();
        return profile;
    }

    if (!std::isnormal(d) || !std::isnormal(step) || !std::isnormal(d / step)) {
        return profile;
    }
    profile.reserve(std::ceil(d / step) + 1);

    mccgeo::GeodLine l(*_wgs84Geod, latLon1, a1, lineMask);
    mccgeo::LatLon curLatLon;
    while (d > currentD) {
        l.position(currentD, &curLatLon, 0);
        auto alt = readAltitude(curLatLon, prec).unwrapOr(0);
        profile.emplace_back(currentD, alt);
        currentD += step;
    }
    auto alt = readAltitude(latLon2, prec).unwrapOr(0);
    profile.emplace_back(d, alt);
    return profile;
}

void HmReader::readAltitudeMatrix(bmcl::ArrayView<double> lats,
                                  bmcl::ArrayView<double> lons,
                                  double* matrix,
                                  double precisionArcSecond,
                                  double defaultValue) const
{
    double* altIt = matrix;
    for (std::size_t yi = 0; yi < lats.size(); yi++) {
        double lat = lats[yi];
        for (std::size_t xi = 0; xi < lons.size(); xi++) {
            *altIt = readAltitude(mccgeo::LatLon(lat, lons[xi]), precisionArcSecond).unwrapOr(defaultValue);
            altIt++;
        }
    }
}

const RcGeod* HmReader::geod() const
{
    return _wgs84Geod.get();
}


EmptyHmReader::EmptyHmReader()
    : HmReader(new RcGeod(mccgeo::wgs84a<double>(), mccgeo::wgs84f<double>()))
{
}

Altitude EmptyHmReader::readAltitude(mccgeo::LatLon latLon, double precisionArcSecond) const
{
    return bmcl::None;
}

const HmReader* EmptyHmReader::clone() const
{
    return this;
}

}
