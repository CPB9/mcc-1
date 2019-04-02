#include "mcc/geo/Vector3D.h"
#include "mcc/geo/Position.h"

#include <bmcl/Math.h>

#include <cmath>

namespace mccgeo
{

constexpr double pi = bmcl::pi<double>();

Vector3D::Vector3D() :
    _x(0.0), _y(0.0), _z(0.0)
{}

Vector3D::Vector3D(double x, double y) :
    _x(x), _y(y), _z(0.0)
{}

Vector3D::Vector3D(double x, double y, double z) :
    _x(x), _y(y), _z(z)
{}

bool Vector3D::isFinite() const
{
    return std::isfinite(_x) &&
           std::isfinite(_y) &&
           std::isfinite(_z);
}

bool Vector3D::isNull() const
{
    if(!isFinite())
        return false;

    return (_x == 0.0) && (_y == 0.0) && (_z == 0.0);
}

double Vector3D::length() const
{
    return sqrt(_x*_x + _y*_y + _z*_z);
}

Position Vector3D::toPositionFromEnu(const Position& basePoint) const {
    double e = x();
    double n = y();
    double u = z();
    double a = 333400. / 3.0;
    double b = 1001879. / 9.0;
    double x = (n / a) + basePoint.latitude();
    double refLat = (basePoint.latitude() + x) / 2.0;
    double y = (e / (cos(refLat * pi / 180.0)*b)) + basePoint.longitude();

    return Position(x, y, u);
}

Position Vector3D::toPositionFromLocal(const Position& basePoint, const Position& directionPoint) const
{
    Vector3D r = directionPoint.toEnu(basePoint);
    double alfa = 0;
    double eps = 1e-6;
    double rad2degr = 180.0 / pi;
    double normR = r.norm();
    if (normR < eps)
        return toPositionFromLocal(basePoint, alfa);
    Vector3D enuNormilizedR = r/normR;
    double cosAlfa = enuNormilizedR.y();
    if (enuNormilizedR.x() > 0.0){
        alfa = acos(cosAlfa)*rad2degr;
        return toPositionFromLocal(basePoint, alfa);
    }
    else{
        alfa = -acos(cosAlfa)*rad2degr;
        return toPositionFromLocal(basePoint, alfa);
    }
}

Position Vector3D::toPositionFromLocal(const Position& basePoint, double angle) const {
    const Vector3D localCord = *this;
    double A = 360.0 - angle;
    double radA = A * pi / 180.0;
    double s = sin(radA);
    double c = cos(radA);
    double enuX = c*localCord.x() - s*localCord.y();
    double enuY = s*localCord.x() + c*localCord.y();
    Vector3D enuThis(enuX, enuY, this->z() + basePoint.altitude());
    return enuThis.toPositionFromEnu(basePoint);
}

double Vector3D::norm() const
{
    return sqrt(x()*x() + y()*y() + z()*z());
}

}
