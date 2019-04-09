#pragma once

#include "mcc/geo/Config.h"

#include <bmcl/Fwd.h>
#include <bmcl/ThreadSafeRefCountable.h>
#include <bmcl/Utils.h>

#include <string>

struct projCtx_t;
typedef struct projCtx_t PJ_CONTEXT;

struct PJconsts;
typedef struct PJconsts PJ;

namespace mccgeo {

class LatLon;
class Coordinate;

enum class CoordinateUnits {
    Degree,
    Meter,
    Kilometer,
    Decimeter,
    Centimeter,
    Millimeter,
    IntNauticalMile,
    IntInch,
    IntFoot,
    IntYard,
    IntStatuteMile,
    IntFathom,
    IntChain,
    IntLink,
    UsSurveyorsInch,
    UsSurveyorsFoot,
    UsSurveyorsYard,
    UsSurveyorsChain,
    UsSurveyorsStatuteMile,
    IndianYard,
    IndianFoot,
    IndianChain,
    None,
};

MCC_GEO_DECLSPEC const char* wgs84Proj4Definition();

class MCC_GEO_DECLSPEC CoordinateConverter : public bmcl::ThreadSafeRefCountable<std::size_t> {
public:
    ~CoordinateConverter();

    static bmcl::Result<bmcl::Rc<CoordinateConverter>, std::string> create();
    static bmcl::Result<bmcl::Rc<CoordinateConverter>, std::string> createFromProj4Definition(const char* def);

    bmcl::Result<bmcl::NoneType, std::string> initFromProj4Definition(const char* def);

    // wgs84 to current
    Coordinate convertForward(const Coordinate& coord) const;
    // current to wgs84
    Coordinate convertInverse(const Coordinate& coord) const;

    bool hasAngularOutput() const;

    const char* definition() const;

    CoordinateUnits units() const;
    CoordinateUnits vunits() const;

private:
    CoordinateConverter(PJ_CONTEXT* ctx, PJ* crs2crs);
    void init();

    struct Transform {
        double inx;
        double iny;
        double outx;
        double outy;
    };

    static Coordinate convert(const Coordinate& coord, const Transform& tr, int direction, PJ* pj);

    PJ_CONTEXT* _ctx;
    mutable PJ* _pj;
    Transform _forward;
    Transform _inverse;
    CoordinateUnits _units;
    CoordinateUnits _vunits;
    bool _hasAngularOutput;
    const char* _def;
};
}
