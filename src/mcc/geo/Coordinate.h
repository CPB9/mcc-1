#pragma once

#include "mcc/geo/Config.h"
#include "mcc/geo/LatLon.h"
#include "mcc/geo/Position.h"
#include "mcc/geo/Vector3D.h"
#include "mcc/geo/Point.h"

namespace mccgeo {

class Coordinate {
public:
    Coordinate(double x, double y, double z = 0, double t = 0)
        : _x(x)
        , _y(y)
        , _z(z)
        , _t(t)
    {
    }

    Coordinate(LatLon latLon)
        : _x(latLon.longitude())
        , _y(latLon.latitude())
        , _z(0)
        , _t(0)
    {
    }

    Coordinate(const Position& pos)
        : _x(pos.longitude())
        , _y(pos.latitude())
        , _z(pos.altitude())
        , _t(0)
    {
    }

    Coordinate(const Vector3D& vec)
        : _x(vec.x())
        , _y(vec.y())
        , _z(vec.z())
        , _t(0)
    {
    }

    Coordinate(Point p)
        : _x(p.x())
        , _y(p.y())
        , _z(0)
        , _t(0)
    {
    }

    double x() const
    {
        return _x;
    }

    double y() const
    {
        return _y;
    }

    double z() const
    {
        return _z;
    }

    double t() const
    {
        return _t;
    }

    double& x()
    {
        return _x;
    }

    double& y()
    {
        return _y;
    }

    double& z()
    {
        return _z;
    }

    double& t()
    {
        return _t;
    }

    void setX(double x)
    {
        _x = x;
    }

    void setY(double y)
    {
        _y = y;
    }

    void setZ(double z)
    {
        _z = z;
    }

    void setT(double t)
    {
        _t = t;
    }

    LatLon latLon() const
    {
        return LatLon(_y, _x);
    }

    Position position() const
    {
        return Position(_y, _x, _z);
    }

    Vector3D vector3d() const
    {
        return Vector3D(_x, _y, _z);
    }

    Point point() const
    {
        return Point(_x, _y);
    }

private:
    double _x;
    double _y;
    double _z;
    double _t;
};

}
