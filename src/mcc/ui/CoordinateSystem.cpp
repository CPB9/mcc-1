#include "mcc/ui/CoordinateSystem.h"
#include "mcc/geo/CoordinateConverter.h"
#include "mcc/geo/Coordinate.h"

namespace mccui {

static const char* unitFromEnum(mccgeo::CoordinateUnits unit)
{
    switch (unit) {
    case mccgeo::CoordinateUnits::Degree:
        return "";
    case mccgeo::CoordinateUnits::Meter:
        return "м";
    case mccgeo::CoordinateUnits::Kilometer:
        return "км";
    case mccgeo::CoordinateUnits::Decimeter:
        return "дм";
    case mccgeo::CoordinateUnits::Centimeter:
        return "см";
    case mccgeo::CoordinateUnits::Millimeter:
        return "мм";
    case mccgeo::CoordinateUnits::IntNauticalMile:
    case mccgeo::CoordinateUnits::IntInch:
    case mccgeo::CoordinateUnits::IntFoot:
    case mccgeo::CoordinateUnits::IntYard:
    case mccgeo::CoordinateUnits::IntStatuteMile:
    case mccgeo::CoordinateUnits::IntFathom:
    case mccgeo::CoordinateUnits::IntChain:
    case mccgeo::CoordinateUnits::IntLink:
    case mccgeo::CoordinateUnits::UsSurveyorsInch:
    case mccgeo::CoordinateUnits::UsSurveyorsFoot:
    case mccgeo::CoordinateUnits::UsSurveyorsYard:
    case mccgeo::CoordinateUnits::UsSurveyorsChain:
    case mccgeo::CoordinateUnits::UsSurveyorsStatuteMile:
    case mccgeo::CoordinateUnits::IndianYard:
    case mccgeo::CoordinateUnits::IndianFoot:
    case mccgeo::CoordinateUnits::IndianChain:
    case mccgeo::CoordinateUnits::None:
        return "";
    }
    return "";
}

CoordinateSystem::CoordinateSystem(const QString& fullName,
                                   const QString& shortName,
                                   const mccgeo::CoordinateConverter* conv)
    : _fullName(fullName)
    , _shortName(shortName)
    , _converter(conv)
{
    _units = unitFromEnum(conv->units());
    _vunits = unitFromEnum(conv->vunits());
}

CoordinateSystem::~CoordinateSystem()
{
}

bool CoordinateSystem::hasAngularUnits() const
{
    return _converter->hasAngularOutput();
}

const char* CoordinateSystem::vunits() const
{
    return _vunits;
}

const char* CoordinateSystem::units() const
{
    return _units;
}

const mccgeo::CoordinateConverter* CoordinateSystem::converter() const
{
    return _converter.get();
}

const char* CoordinateSystem::definition() const
{
    return _converter->definition();
}

const QString& CoordinateSystem::shortName() const
{
    return _shortName;
}

const QString& CoordinateSystem::fullName() const
{
    return _fullName;
}

mccgeo::Coordinate CoordinateSystem::convertForward(const mccgeo::Coordinate& coord) const
{
    return _converter->convertForward(coord);
}

mccgeo::Coordinate CoordinateSystem::convertInverse(const mccgeo::Coordinate& coord) const
{
    return _converter->convertInverse(coord);
}
}
