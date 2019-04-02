#pragma once

#include "mcc/geo/Config.h"
#include "mcc/geo/LatLon.h"

namespace mccgeo {

class Vector3D;

class MCC_GEO_DECLSPEC Position : public LatLon {
public:
    Position();
    Position(double latitude, double longitude, double altitude = 0.0);
    explicit Position(const LatLon& latLon, double altitude = 0.0);

    bool isFinite() const;
    bool isNull() const;

    const LatLon& latLon() const;
    double altitude() const;

    LatLon& latLon();
    double& altitude();

    void setLatLon(const LatLon& latLon);
    void setAltitude(double altitude);

    // Converting from WGS84
    Vector3D toEnu(const Position& basePosition = Position()) const; // returns local ENU coords relatively p0
    Vector3D toLocal(const Position& basePoint, const Position& directionPoint) const;
    Vector3D toLocal(const Position& basePoint, double angle) const;

    friend const Position operator+(const Position &left, const Position &right);
    friend const Position operator-(const Position &left, const Position &right);

private:
    double _altitude;
};

inline Position::Position()
    : LatLon(0.0, 0.0)
    , _altitude(0.0)
{
}

inline Position::Position(double latitude, double longitude, double altitude)
    : LatLon(latitude, longitude)
    , _altitude(altitude)
{
}

inline Position::Position(const LatLon &latLon, double altitude) :
    LatLon(latLon),
    _altitude(altitude)
{}

inline const LatLon& Position::latLon() const
{
    return *this;
}

inline LatLon& Position::latLon()
{
    return *this;
}

inline void Position::setLatLon(const LatLon& latLon)
{
    LatLon::operator=(latLon);
}


inline double Position::altitude() const
{
    return _altitude;
}

inline double& Position::altitude()
{
    return _altitude;
}

inline void Position::setAltitude(double altitude)
{
    _altitude = altitude;
}

inline const Position operator+(const Position &left, const Position &right)
{
    return Position(left.latitude()  + right.latitude(),
                    left.longitude() + right.longitude(),
                    left.altitude()  + right.altitude());
}

inline const Position operator-(const Position &left, const Position &right)
{
    return Position(left.latitude()  - right.latitude(),
                    left.longitude() - right.longitude(),
                    left.altitude()  - right.altitude());
}

MCC_GEO_DECLSPEC bool operator==(const Position& left, const Position& right);
MCC_GEO_DECLSPEC bool operator!=(const Position& left, const Position& right);

class MCC_GEO_DECLSPEC PositionAndDistance : public Position {
public:
    PositionAndDistance(double lat, double lon, double alt, double dist);
    PositionAndDistance(const LatLon& latLon, double alt, double dist);
    PositionAndDistance(const Position& p, double dist);
    PositionAndDistance();

    const Position& position() const;
    double distance() const;

    Position& position();
    double& distance();

    void setPosition(const Position& position);
    void setDistance(double distance);

private:
    double _distance;
};

inline PositionAndDistance::PositionAndDistance(double lat, double lon, double alt, double dist)
    : Position(lat, lon, alt)
    , _distance(dist)
{
}

inline PositionAndDistance::PositionAndDistance(const LatLon& latLon, double alt, double dist)
    : Position(latLon, alt)
    , _distance(dist)
{
}

inline PositionAndDistance::PositionAndDistance(const Position& p, double dist)
    : Position(p)
    , _distance(dist)
{
}

inline PositionAndDistance::PositionAndDistance()
    : Position(0.0, 0.0, 0.0)
    , _distance(0.0)
{
}

inline const Position& PositionAndDistance::position() const
{
    return *this;
}

inline Position& PositionAndDistance::position()
{
    return *this;
}

inline void PositionAndDistance::setPosition(const Position& position)
{
    Position::operator=(position);
}

inline double PositionAndDistance::distance() const
{
    return _distance;
}

inline double& PositionAndDistance::distance()
{
    return _distance;
}

inline void PositionAndDistance::setDistance(double distance)
{
    _distance = distance;
}

}

