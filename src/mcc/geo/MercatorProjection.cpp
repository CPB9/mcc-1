#include "mcc/geo/MercatorProjection.h"
#include "mcc/geo/LatLon.h"

#include <bmcl/Math.h>
#include <bmcl/DoubleEq.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdint>

namespace mccgeo {

using bmcl::radiansToDegrees;
using bmcl::degreesToRadians;

constexpr double pi = bmcl::pi<double>();
constexpr double pi_2 = bmcl::halfPi<double>();
constexpr double pi_4 = bmcl::fourthPi<double>();

double MercatorProjection::calcDistance(double lat1, double lon1, double lat2, double lon2) const
{
    double slat = std::sin((degreesToRadians(lat2 - lat1)) / 2);
    double slon = std::sin(degreesToRadians((lon2 - lon1) / 2));
    double a = std::fma(slon * slon, std::cos(degreesToRadians(lat1)) * std::cos(degreesToRadians(lat2)), slat * slat);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return _a * c;
}

double MercatorProjection::latitudeToRelativeOffset(double lat) const
{
    lat = degreesToRadians(lat);
    double esinlat = _e * std::sin(lat);
    return 0.5 * std::log(std::tan(std::fma(0.5, lat, pi_4)) * std::pow((1 - esinlat) / (1 + esinlat), _e / 2)) / pi;
}

double MercatorProjection::latitudeToY(double lat) const
{
    lat = degreesToRadians(lat);
    double esinlat = _e * std::sin(lat);
    return _a * std::log(std::tan(std::fma(0.5, lat, pi_4)) * std::pow((1 - esinlat) / (1 + esinlat), _e / 2));
}

// c wiki OSM

double MercatorProjection::tsToLatitude(double ts) const
{
    double lat = std::fma(-2, std::atan(ts), pi_2);
    double dlat = 1.0;
    double prec = 0.000000001;
    int i = 0;
    while ((std::abs(dlat) > prec) && (i < 15)) {
        double esinlat = _e * std::sin(lat);
        dlat = std::fma(-2, std::atan(ts * std::pow((1 - esinlat) / (1 + esinlat), _e / 2)), pi_2 - lat);
        lat += dlat;
        i++;
    }
    return radiansToDegrees(lat);
}

double MercatorProjection::scalingFactorFromLatitude(double lat) const
{
    lat = degreesToRadians(lat);
    return std::sqrt(1 - std::pow(_e * std::sin(lat), 2)) / std::cos(lat);
}

double MercatorProjection::ellipseCircumference(double a, double b)
{
    // аппроксимация периметра эллипса по Бесселю
    double h = std::pow((a - b) / (a + b), 2);
    double c[15] = {16.0, 4.0, 2.56, 2.0408163265306123, 1.7777777777777777, 1.6198347107438016, 1.514792899408284,
                    1.44, 1.3840830449826989, 1.3407202216066483, 1.3061224489795917, 1.277882797731569, 1.2544,
                    1.2345679012345678, 1.2175980975029725};
    double hh = 0.25 * h;
    double s = 1;
    for (int i = 0; i < 15; i++) {
        s += hh;
        hh *= h / c[i];
    }
    return pi * (a + b) * s;
}

const double spherA = 6378137;
const double spherB = 6378137;
const double ellA = 6378137;
const double ellB = 6356752.314245;

void MercatorProjection::init(MercatorProjection::ProjectionType type)
{
    if (type == ProjectionType::EllipticalMercator) {
        init(ellA, ellB);
    } else {
        init(spherA, spherB);
    }
}

bool MercatorProjection::isA(MercatorProjection::ProjectionType type) const
{
    if (type == SphericalMercator) {
        return bmcl::doubleEq(spherA, _a) && bmcl::doubleEq(spherB, _b);
    } else if (type == EllipticalMercator) {
        return bmcl::doubleEq(ellA, _a) && bmcl::doubleEq(ellB, _b);
    }
    return false;
}

void MercatorProjection::init(double a, double b)
{
    _a = std::max(a, b);
    _b = std::min(a, b);
    _e = std::sqrt(1 - std::pow(_b / _a, 2));
    _equatorCircumference = ellipseCircumference(_a, _b);
    _maxLatitude = degreesToRadians(yToLatitude(_equatorCircumference / 2));
}

double MercatorProjection::calcDistance(const LatLon& from, const LatLon& to) const
{
    return calcDistance(from.latitude(), from.longitude(), to.latitude(), to.longitude());
}

double MercatorProjection::relativeOffsetToLatitude(double offset) const
{
    return tsToLatitude(std::exp(-offset / 0.5 * pi));
}

double MercatorProjection::yToLatitude(double y) const
{
    return tsToLatitude(std::exp(-y / _a));
}

double MercatorProjection::scalingFactorFromY(double y) const
{
    return std::cosh(2 * pi * y / _equatorCircumference);
}

double MercatorProjection::maxLatitude() const
{
    return radiansToDegrees(_maxLatitude);
}

double MercatorProjection::longitudeToX(double lon) const
{
    return _a * degreesToRadians(lon);
}

double MercatorProjection::longitudeToRelativeOffset(double lon) const
{
    return degreesToRadians(lon) / pi;
}

double MercatorProjection::xToLongitude(double x) const
{
    return radiansToDegrees(x / _a);
}

double MercatorProjection::relativeOffsetToLongitude(double relativeOffset) const
{
    return radiansToDegrees(relativeOffset * pi);
}
}
