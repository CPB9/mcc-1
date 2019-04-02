#pragma once

#include "mcc/geo/Config.h"
#include "mcc/geo/LatLon.h"
#include "mcc/geo/detail/geodesic.h"

namespace mccgeo {

enum class GeodFlags : unsigned {
    NoFlags = detail::GEOD_NOFLAGS,
    ArcMode = detail::GEOD_ARCMODE,
    LongUnroll = detail::GEOD_LONG_UNROLL,
};

enum class GeodMask : unsigned {
    None = detail::GEOD_NONE,
    Latitude = detail::GEOD_LATITUDE,
    Longitude = detail::GEOD_LONGITUDE,
    Azimuth = detail::GEOD_AZIMUTH,
    Distance = detail::GEOD_DISTANCE,
    DistanceIn = detail::GEOD_DISTANCE_IN,
    RedicedLength = detail::GEOD_REDUCEDLENGTH,
    GeodesicScale = detail::GEOD_GEODESICSCALE,
    Area = detail::GEOD_AREA,
    All = detail::GEOD_ALL,
};

class Geod {
public:
    Geod(double a, double f)
    {
        init(a, f);
    }

    void init(double a, double f)
    {
        detail::geod_init(&_geod, a, f);
    }

    void direct(double lat1, double lon1, double azi1, double s12, double* plat2, double* plon2, double* pazi2) const
    {
        detail::geod_direct(&_geod, lat1, lon1, azi1, s12, plat2, plon2, pazi2);
    }

    void direct(LatLon latLon1, double azi1, double s12, LatLon* platLon2, double* pazi2) const
    {
        detail::geod_direct(&_geod, latLon1.latitude(), latLon1.longitude(), azi1, s12,
                            &platLon2->latitude(), &platLon2->longitude(), pazi2);
    }

    double genDirect(double lat1, double lon1, double azi1, GeodFlags flags, double s12_a12,
                     double* plat2, double* plon2, double* pazi2,
                     double* ps12, double* pm12, double* pM12, double* pM21, double* pS12) const
    {
        return detail::geod_gendirect(&_geod, lat1, lon1, azi1, (unsigned)flags, s12_a12, plat2, plon2, pazi2,
                                      ps12, pm12, pM12, pM21, pS12);
    }

    double genDirect(LatLon latLon1, double azi1, GeodFlags flags, double s12_a12, LatLon* platLon2, double* pazi2,
                     double* ps12, double* pm12, double* pM12, double* pM21, double* pS12) const
    {
        return detail::geod_gendirect(&_geod, latLon1.latitude(), latLon1.longitude(),
                                      azi1, (unsigned)flags, s12_a12,
                                      &platLon2->latitude(), &platLon2->longitude(), pazi2,
                                      ps12, pm12, pM12, pM21, pS12);
    }

    void inverse(double lat1, double lon1, double lat2, double lon2, double* ps12, double* pazi1, double* pazi2) const
    {
        detail::geod_inverse(&_geod, lat1, lon1, lat2, lon2, ps12, pazi1, pazi2);
    }

    void inverse(LatLon latLon1, LatLon latLon2, double* ps12, double* pazi1, double* pazi2) const
    {
        detail::geod_inverse(&_geod, latLon1.latitude(), latLon1.longitude(),
                             latLon2.latitude(), latLon2.longitude(), ps12, pazi1, pazi2);
    }

    double genInverse(double lat1, double lon1, double lat2, double lon2,
                      double* ps12, double* pazi1, double* pazi2,
                      double* pm12, double* pM12, double* pM21, double* pS12) const
    {
        return detail::geod_geninverse(&_geod, lat1, lon1, lat2, lon2, ps12, pazi1, pazi2, pm12, pM12, pM21, pS12);
    }

    double genInverse(LatLon latLon1, LatLon latLon2, double* ps12, double* pazi1, double* pazi2,
                      double* pm12, double* pM12, double* pM21, double* pS12) const
    {
        return detail::geod_geninverse(&_geod, latLon1.latitude(), latLon1.longitude(),
                                       latLon2.latitude(), latLon2.longitude(),
                                       ps12, pazi1, pazi2, pm12, pM12, pM21, pS12);
    }

private:
    friend class GeodLine;

    struct detail::geod_geodesic _geod;
};

class GeodLine {
public:
    GeodLine(const Geod& geod, LatLon latLon1, double azi1, GeodMask caps)
    {
        init(geod, latLon1, azi1, caps);
    }

    GeodLine(const Geod& geod, LatLon latLon1, double azi1, double s12, GeodMask caps)
    {
        initDirect(geod, latLon1.latitude(), latLon1.longitude(), azi1, s12, caps);
    }

    GeodLine(const Geod& geod, LatLon latLon1, double azi1, GeodFlags flags, double s12_a12, GeodMask caps)
    {
        initGenDirect(geod, latLon1.latitude(), latLon1.longitude(), azi1, flags, s12_a12, caps);
    }

    GeodLine(const Geod& geod, LatLon latLon1, LatLon latLon2, GeodMask caps)
    {
        initInverse(geod, latLon1.latitude(), latLon1.longitude(), latLon2.latitude(), latLon2.longitude(), caps);
    }

    void init(const Geod& geod, double lat1, double lon1, double azi1, GeodMask caps)
    {
        detail::geod_lineinit(&_line, &geod._geod, lat1, lon1, azi1, (unsigned)caps);
    }

    void init(const Geod& geod, LatLon latLon1, double azi1, GeodMask caps)
    {
        detail::geod_lineinit(&_line, &geod._geod, latLon1.latitude(), latLon1.longitude(), azi1, (unsigned)caps);
    }

    void initDirect(const Geod& geod, double lat1, double lon1, double azi1, double s12, GeodMask caps)
    {
        detail::geod_directline(&_line, &geod._geod, lat1, lon1, azi1, s12, (unsigned)caps);
    }

    void initDirect(const Geod& geod, LatLon latLon1, double azi1, double s12, GeodMask caps)
    {
        detail::geod_directline(&_line, &geod._geod, latLon1.latitude(), latLon1.longitude(), azi1, s12, (unsigned)caps);
    }

    void initGenDirect(const Geod& geod, double lat1, double lon1, double azi1,
                       GeodFlags flags, double s12_a12, GeodMask caps)
    {
        detail::geod_gendirectline(&_line, &geod._geod, lat1, lon1, azi1, (unsigned)flags, s12_a12, (unsigned)caps);
    }

    void initGenDirect(const Geod& geod, LatLon latLon1, double azi1,
                       GeodFlags flags, double s12_a12, GeodMask caps)
    {
        detail::geod_gendirectline(&_line, &geod._geod, latLon1.latitude(), latLon1.longitude(),
                                   azi1, (unsigned)flags, s12_a12, (unsigned)caps);
    }

    void initInverse(const Geod& geod, double lat1, double lon1, double lat2, double lon2, GeodMask caps)
    {
        detail::geod_inverseline(&_line, &geod._geod, lat1, lon1, lat2, lon2, (unsigned)caps);
    }

    void initInverse(const Geod& geod, LatLon latLon1, LatLon latLon2, GeodMask caps)
    {
        detail::geod_inverseline(&_line, &geod._geod, latLon1.latitude(), latLon1.longitude(),
                                 latLon2.latitude(), latLon2.longitude(), (unsigned)caps);
    }

    void position(double s12, double* plat2, double* plon2, double* pazi2) const
    {
        detail::geod_position(&_line, s12, plat2, plon2, pazi2);
    }

    void position(double s12, LatLon* platLon2, double* pazi2) const
    {
        detail::geod_position(&_line, s12, &platLon2->latitude(), &platLon2->longitude(), pazi2);
    }

    double genPosition(GeodFlags flags, double s12_a12, double* plat2, double* plon2, double* pazi2,
                       double* ps12, double* pm12, double* pM12, double* pM21, double* pS12) const
    {
        return detail::geod_genposition(&_line, (unsigned)flags, s12_a12, plat2, plon2, pazi2,
                                        ps12, pm12, pM12, pM21, pS12);
    }

    double genPosition(GeodFlags flags, double s12_a12, LatLon* platLon2, double* pazi2,
                       double* ps12, double* pm12, double* pM12, double* pM21, double* pS12) const
    {
        return detail::geod_genposition(&_line, (unsigned)flags, s12_a12, &platLon2->latitude(), &platLon2->longitude(),
                                        pazi2, ps12, pm12, pM12, pM21, pS12);
    }

    void setDistance(double s13)
    {
        detail::geod_setdistance(&_line, s13);
    }

    void genSetDistance(GeodFlags flags, double s13_a13)
    {
        detail::geod_gensetdistance(&_line, (unsigned)flags, s13_a13);
    }

private:
    detail::geod_geodesicline _line;
};

#define DEFINE_GEOD_ENUM_OPS(name)                      \
                                                        \
constexpr name operator|(name left, name right)         \
{                                                       \
    return name(unsigned(left) | unsigned(right));      \
}                                                       \
                                                        \
constexpr name operator&(name left, name right)         \
{                                                       \
    return name(unsigned(left) & unsigned(right));      \
}                                                       \
                                                        \
constexpr name operator^(name left, name right)         \
{                                                       \
    return name(unsigned(left) ^ unsigned(right));      \
}                                                       \
                                                        \
constexpr name operator~(name value)                    \
{                                                       \
    return name(~unsigned(value));                      \
}                                                       \
                                                        \
inline name& operator|=(name& left, name right)         \
{                                                       \
    left = name(unsigned(left) | unsigned(right));      \
    return left;                                        \
}                                                       \
inline name& operator&=(name& left, name right)         \
{                                                       \
    left = name(unsigned(left) & unsigned(right));      \
    return left;                                        \
}                                                       \
inline name& operator^=(name& left, name right)         \
{                                                       \
    left = name(unsigned(left) ^ unsigned(right));      \
    return left;                                        \
}

DEFINE_GEOD_ENUM_OPS(GeodFlags);
DEFINE_GEOD_ENUM_OPS(GeodMask);

#undef DEFINE_GEOD_OPS
}
