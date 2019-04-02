#include "mcc/geo/Bbox.h"

namespace mccgeo {

Bbox::Bbox()
    : _topLeft(0.0, 0.0)
    , _bottomRight(0.0, 0.0)
{
}

Bbox::Bbox(const LatLon& single)
    : _topLeft(single)
    , _bottomRight(single)
{
}

Bbox::Bbox(const LatLon& topLeft, const LatLon& bottomRight)
    : _topLeft(topLeft)
    , _bottomRight(bottomRight)
{
}

LatLon Bbox::center() const
{
    double dlat = (_topLeft.latitude() - _bottomRight.latitude()) / 2;
    double dlon = (_bottomRight.longitude() - _topLeft.longitude()) / 2;
    return LatLon(_bottomRight.latitude() + dlat, _topLeft.longitude() + dlon);
}

bool Bbox::isPoint() const //FIXME: doubleEq
{
    return _topLeft.latitude() == _bottomRight.latitude() && _topLeft.longitude() == _bottomRight.longitude();
}

const LatLon& Bbox::topLeft() const
{
    return _topLeft;
}

LatLon Bbox::topRight() const
{
    return LatLon(_bottomRight.latitude(), _topLeft.longitude());
}

void Bbox::setTopLeft(const LatLon& topLeft)
{
    _topLeft = topLeft;
}

const LatLon& Bbox::bottomRight() const
{
    return _bottomRight;
}

LatLon Bbox::bottomLeft() const
{
    return LatLon(_topLeft.latitude(), _bottomRight.longitude());
}

void Bbox::setBottomRight(const LatLon& bottomRight)
{
    _bottomRight = bottomRight;
}

double Bbox::left() const
{
    return _topLeft.latitude();
}

void Bbox::setLeft(double left)
{
    _topLeft.latitude() = left;
}

double Bbox::right() const
{
    return _bottomRight.latitude();
}

void Bbox::setRight(double right)
{
    _bottomRight.latitude() = right;
}

double Bbox::top() const
{
    return _topLeft.longitude();
}

void Bbox::setTop(double top)
{
    _topLeft.longitude() = top;
}

double Bbox::bottom() const
{
    return _bottomRight.longitude();
}

void Bbox::setBottom(double bottom)
{
    _bottomRight.longitude() = bottom;
}
}