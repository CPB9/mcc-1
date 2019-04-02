#pragma once

#include "mcc/geo/Config.h"

namespace mccgeo {

class LatLon {
public:
    LatLon();
    LatLon(double latitude, double longitude);

    bool isValid() const;

    double latitude() const;
    double longitude() const;

    double& latitude();
    double& longitude();

    void setLatitude(double latitude);
    void setLongitude(double longitude);

    friend bool operator!=(const LatLon &left, const LatLon &right);
    friend const LatLon operator+(const LatLon &left, const LatLon &right);
    friend const LatLon operator-(const LatLon &left, const LatLon &right);

private:
    double _latitude;
    double _longitude;
};

inline LatLon::LatLon()
    : _latitude(0.0)
    , _longitude(0.0)
{
}

inline LatLon::LatLon(double latitude, double longitude)
    : _latitude(latitude)
    , _longitude(longitude)
{
}

inline bool LatLon::isValid() const
{
    return _latitude >= -90.0 && _latitude <= 90.0 && _longitude >= -180.0 && _longitude <= 180.0;
}

inline double LatLon::latitude() const
{
    return _latitude;
}

inline double LatLon::longitude() const
{
    return _longitude;
}

inline double& LatLon::latitude()
{
    return _latitude;
}

inline double& LatLon::longitude()
{
    return _longitude;
}

inline void LatLon::setLatitude(double latitude)
{
    _latitude = latitude;
}

inline void LatLon::setLongitude(double longitude)
{
    _longitude = longitude;
}

MCC_GEO_DECLSPEC bool operator==(const LatLon& left, const LatLon& right);
}
