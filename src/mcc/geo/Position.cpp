#include "mcc/geo/Position.h"
#include "mcc/geo/Vector3D.h"

#include <bmcl/Math.h>
#include <bmcl/DoubleEq.h>

#include <cmath>

namespace mccgeo {

constexpr double pi = bmcl::pi<double>();

bool operator==(const Position& left, const Position& right)
{
    return left.latLon() == right.latLon()
        && bmcl::doubleEq(left.altitude(), right.altitude());
}

bool operator!=(const Position& left, const Position& right)
{
    return !(left == right);
}

bool Position::isFinite() const
{
    return std::isfinite(latitude()) &&
           std::isfinite(longitude()) &&
           std::isfinite(_altitude);
}

bool Position::isNull() const
{
    if(!isFinite())
        return false;

    return (latitude() == 0.0) && (longitude() == 0.0) && (_altitude == 0.0);
}

Vector3D Position::toEnu(const Position &basePosition) const {
    double e = longitude() - basePosition.longitude();
    double n = latitude() - basePosition.latitude();
    double u = altitude();
    double refLat = 0.5 * (latitude() + basePosition.latitude());
    double nm = n * 333400.0 / 3.0;
    double em = e * 1001879.0 * cos(refLat * pi / 180.0) / 9.0;
    return Vector3D(em, nm, u);
}

Vector3D Position::toLocal(const Position& basePoint, const Position& directionPoint) const
{
    Vector3D r = directionPoint.toEnu(basePoint);
    double alfa = 0.0;
    double eps = 1e-6;
    double rad2degr = 180.0 / pi;
    double normR = r.norm();
    if (normR < eps)
        return toLocal(basePoint, alfa);
    Vector3D enuNormilizedR = r/normR;
    double cosAlfa = enuNormilizedR.y();
    if (enuNormilizedR.x() > 0.0){
        alfa = acos(cosAlfa)*rad2degr;
        return toLocal(basePoint, alfa);
    }
    else{
        alfa = -acos(cosAlfa)*rad2degr;
        return toLocal(basePoint, alfa);
    }
}

Vector3D Position::toLocal(const Position& basePoint, double angle) const{
    Vector3D enuCoord = this->toEnu(basePoint);
    double A = 360.0 - angle;
    double radA = A * pi / 180.0;
    double s = sin(-radA);
    double c = cos(-radA);
    double x = c*enuCoord.x() - s*enuCoord.y();
    double y = s*enuCoord.x() + c*enuCoord.y();
    return Vector3D(x, y, this->altitude() - basePoint.altitude());
}

}
