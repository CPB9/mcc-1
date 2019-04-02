#include "mcc/ui/CoordinateFormat.h"

namespace mccui {

CoordinateFormat::CoordinateFormat(const char* units)
    : _data(units)
{
}

CoordinateFormat::CoordinateFormat(AngularFormat fmt)
    : _data(fmt)
{
}

bool CoordinateFormat::isAngular() const
{
    return _data.isFirst();
}

bool CoordinateFormat::isLinear() const
{
    return _data.isSecond();
}

AngularFormat CoordinateFormat::unwrapAngular() const
{
    return _data.unwrapFirst();
}

const char* CoordinateFormat::unwrapLinear() const
{
    return _data.unwrapSecond();
}

void CoordinateFormat::setLinear(const char* units)
{
    _data = units;
}

void CoordinateFormat::setAngular(AngularFormat fmt)
{
    _data = fmt;
}
}
