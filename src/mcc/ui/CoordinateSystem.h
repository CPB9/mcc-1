#pragma once

#include "mcc/Config.h"
#include "mcc/geo/Fwd.h"
#include "mcc/ui/Rc.h"

#include <QString>

namespace mccui {

class MCC_UI_DECLSPEC CoordinateSystem {
public:
    CoordinateSystem(const QString& fullName,
                     const QString& shortName,
                     const mccgeo::CoordinateConverter* conv);

    ~CoordinateSystem();

    const QString& fullName() const;
    const QString& shortName() const;
    const char* definition() const;

    const mccgeo::CoordinateConverter* converter() const;
    mccgeo::Coordinate convertForward(const mccgeo::Coordinate& coord) const;
    mccgeo::Coordinate convertInverse(const mccgeo::Coordinate& coord) const;

    bool hasAngularUnits() const;
    const char* units() const;
    const char* vunits() const;

private:
    QString _fullName;
    QString _shortName;
    Rc<const mccgeo::CoordinateConverter> _converter;
    const char* _units;
    const char* _vunits;
};
}
