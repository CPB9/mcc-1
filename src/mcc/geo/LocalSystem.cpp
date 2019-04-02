#include "mcc/geo/LocalSystem.h"
#include "mcc/geo/Vector3D.h"

#include <bmcl/Math.h>

#include <cmath>

namespace mccgeo
{

constexpr double pi = bmcl::pi<double>();

LocalSystem::LocalSystem(const Position& centerPoint, double angle) :
    _centerPoint(centerPoint),
    _angle(angle)
{
}

LocalSystem::LocalSystem(const Position& centerPoint, const Position& directionPoint) :
    _centerPoint(centerPoint)
{
    _angle = angleFromDirection(directionPoint);
}

Vector3D LocalSystem::toLocal(const Position& globalPosition) const
{
    return globalPosition.toLocal(_centerPoint, _angle);
}

Vector3D LocalSystem::toLocal(const LatLon& globalLatLon) const
{
    return toLocal(Position(globalLatLon));
}

Position LocalSystem::toGlobal(const Vector3D& localCoordinate) const
{
    return localCoordinate.toPositionFromLocal(_centerPoint, _angle);
}

Position LocalSystem::directionPoint() const
{
    double A = 360.0 + 90.0 - _angle;
    double radA = A * pi / 180.0;
    double s = sin(radA);
    double c = cos(radA);
    double factor = 100.0;
    Vector3D enuVec(c * factor, s * factor, centerPoint().altitude());

    return enuVec.toPositionFromEnu(centerPoint());
}

void LocalSystem::setSystem(const Position& centerPoint, double angle)
{
    setCenterPoint(centerPoint);
    setAngle(angle);
}

void LocalSystem::setSystem(const Position& centerPoint, const Position& directionPoint)
{
    setCenterPoint(centerPoint);
    setDirectionPoint(directionPoint);
}

void LocalSystem::setDirectionPoint(const Position& directionPoint)
{
    _angle = angleFromDirection(directionPoint);
}

double LocalSystem::maxRadius(double mistakeMeters) const
{
    double k = 1001879.0/ 9.0;
    double b = 333400.0 / 3.0;
    double latLimit = 5.0;
    double lat0 = fabs(centerPoint().latitude());
    if (lat0 < latLimit) {
        lat0 = latLimit;
    }
    return fabs(b*sqrt(mistakeMeters/(k*sin(lat0*pi/180)*pi/180)));
}

double LocalSystem::distanceLimit()
{
    // Half of the Earth equatorial circumference, m
    return 6378100.0 * pi;
}

static double normalizeAngle360Deg(double angleDeg)
{
    if(angleDeg > 0.0)
    {
        angleDeg = std::fmod(angleDeg, 360.0);
    }
    else if(angleDeg < 0.0)
    {
        angleDeg = std::fmod(angleDeg, 360.0) + 360.0;
    }
    return angleDeg;
}


double LocalSystem::angleFromDirection(const Position& directionPoint) const
{
    double eps = 1e-6;
    double rad2degr = 180.0 / pi;
    Vector3D enuR = directionPoint.toEnu(_centerPoint);
    double normR = enuR.norm();
    if (normR < eps)
        return 0.0;
    Vector3D enuNormilizedR = enuR/normR;
    double cosAlfa = enuNormilizedR.y();
    if (enuNormilizedR.x() > 0.0)
        return normalizeAngle360Deg(acos(cosAlfa)*rad2degr);
    else
        return normalizeAngle360Deg(-acos(cosAlfa)*rad2degr);
}

Position LocalSystem::centerPoint() const {return _centerPoint;}
double LocalSystem::angle() const {return _angle;}
void LocalSystem::setCenterPoint(const Position& centerPoint) {_centerPoint = centerPoint;}
void LocalSystem::setAngle(double angle) {_angle = angle;}
}
