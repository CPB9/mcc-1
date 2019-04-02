#pragma once

#include "mcc/geo/Config.h"
#include "mcc/geo/Fwd.h"

namespace mccgeo {

class MCC_GEO_DECLSPEC MercatorProjection {
public:
    enum ProjectionType { EllipticalMercator, SphericalMercator };

    inline MercatorProjection();
    inline MercatorProjection(double a, double b);
    inline MercatorProjection(ProjectionType type);

    double calcDistance(double lat1, double lon1, double lat2, double lon2) const;
    double calcDistance(const LatLon& from, const LatLon& to) const;
    double longitudeToX(double lon) const;
    double longitudeToRelativeOffset(double lon) const;
    double xToLongitude(double x) const;
    double relativeOffsetToLatitude(double relativeOffset) const;
    double relativeOffsetToLongitude(double relativeOffset) const;
    double latitudeToRelativeOffset(double lat) const;
    double latitudeToY(double lat) const;
    double yToLatitude(double y) const;
    double scalingFactorFromLatitude(double lat) const;
    double scalingFactorFromY(double y) const;
    inline double mapWidth() const;
    double maxLatitude() const;
    inline double majorAxis() const;
    inline double minorAxis() const;
    inline double equatorCircumference() const;
    inline double parallelCircumference(double lat) const;

    bool isA(ProjectionType type) const;

private:
    double tsToLatitude(double ts) const;
    static double ellipseCircumference(double a, double b);
    void init(ProjectionType type);
    void init(double a, double b);

    double _a; // большая полуось
    double _b; // малая полуось
    double _e; // эксцентриситет
    double _maxLatitude;
    double _equatorCircumference;
};

inline MercatorProjection::MercatorProjection()
{
    init(SphericalMercator);
}

inline MercatorProjection::MercatorProjection(double a, double b)
{
    init(a, b);
}

inline MercatorProjection::MercatorProjection(MercatorProjection::ProjectionType type)
{
    init(type);
}

inline double MercatorProjection::mapWidth() const
{
    return _equatorCircumference;
}

inline double MercatorProjection::majorAxis() const
{
    return _a;
}

inline double MercatorProjection::minorAxis() const
{
    return _b;
}

inline double MercatorProjection::equatorCircumference() const
{
    return _equatorCircumference;
}

inline double MercatorProjection::parallelCircumference(double lat) const
{
    return _equatorCircumference / scalingFactorFromLatitude(lat);
}
}
