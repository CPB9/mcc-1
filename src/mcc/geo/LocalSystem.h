#pragma once

#include "mcc/geo/Config.h"
#include "mcc/geo/Position.h"

namespace mccgeo
{

class Vector3D;

class MCC_GEO_DECLSPEC LocalSystem
{
public:
    LocalSystem(const Position& centerPoint, double angle);
    LocalSystem(const Position& centerPoint, const Position& directionPoint);

    Vector3D toLocal(const Position& globalPosition) const;
    Vector3D toLocal(const LatLon& globalLatLon) const;
    Position toGlobal(const Vector3D& localCoordinate) const;

    Position centerPoint() const;
    double angle() const;
    Position directionPoint() const;

    void setSystem(const Position& centerPoint, double angle);
    void setSystem(const Position& centerPoint, const Position& directionPoint);

    void setCenterPoint(const Position& centerPoint);
    void setAngle(double angle);
    void setDirectionPoint(const Position& directionPoint);

    double maxRadius(double mistakeMeters = 1.0) const;
    static double distanceLimit();

private:
    double angleFromDirection(const Position& directionPoint) const;

private:
    Position    _centerPoint;   // Position in WGS84
    double      _angle;         // Angle from North-direction, clockwise positive
};
}
