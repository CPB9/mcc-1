#pragma once

#include "mcc/geo/Config.h"

#include <math.h>

namespace mccgeo
{

class Position;
class LatLon;

class MCC_GEO_DECLSPEC Vector3D
{
public:
    Vector3D();
    Vector3D(double x, double y);
    Vector3D(double x, double y, double z);

    bool isFinite() const;
    bool isNull() const;

    double x() const {return _x;}
    double y() const {return _y;}
    double z() const {return _z;}

    void setX(double x) {_x = x;}
    void setY(double y) {_y = y;}
    void setZ(double z) {_z = z;}

    double length() const;

    // Convertion to WGS84
    Position toPositionFromEnu(const Position& basePoint) const; // returns gps position relatively gps0
    Position toPositionFromLocal(const Position& basePoint, const Position& directionPoint) const;
    Position toPositionFromLocal(const Position& basePoint, double angle) const;

    double norm() const;

    friend bool operator==(const Vector3D &v1, const Vector3D &v2);
    friend bool operator!=(const Vector3D &v1, const Vector3D &v2);
    friend const Vector3D operator+(const Vector3D &v1, const Vector3D &v2);
    friend const Vector3D operator-(const Vector3D &v1, const Vector3D &v2);
    friend const Vector3D operator-(const Vector3D &vector);
    friend const Vector3D operator/(const Vector3D &vector, double a);
    friend const Vector3D operator*(const Vector3D &vector, double a);

private:
    double _x;
    double _y;
    double _z;
};

inline bool operator==(const Vector3D &v1, const Vector3D &v2)
{
    return v1._x == v2._x && v1._y == v2._y && v1._z == v2._z;
}

inline bool operator!=(const Vector3D &v1, const Vector3D &v2)
{
    return v1._x != v2._x || v1._y != v2._y || v1._z != v2._z;
}

inline const Vector3D operator+(const Vector3D &v1, const Vector3D &v2)
{
    return Vector3D(v1._x + v2._x, v1._y + v2._y, v1._z + v2._z);
}

inline const Vector3D operator-(const Vector3D &v1, const Vector3D &v2)
{
    return Vector3D(v1._x - v2._x, v1._y - v2._y, v1._z - v2._z);
}

inline const Vector3D operator-(const Vector3D &vector)
{
    return Vector3D(-vector._x, -vector._y, -vector._z);
}

inline const Vector3D operator/(const Vector3D &v1, double a)
{
    return Vector3D(v1._x / a, v1._y / a, v1._z / a);
}

inline const Vector3D operator*(const Vector3D &v1, double a)
{
    return Vector3D(v1._x * a, v1._y * a, v1._z * a);
}

}
