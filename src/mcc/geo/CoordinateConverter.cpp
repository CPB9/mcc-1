#include "mcc/geo/CoordinateConverter.h"
#include "mcc/geo/Coordinate.h"
#include "mcc/geo/LatLon.h"

#include <proj.h>

#include <bmcl/Result.h>
#include <bmcl/Rc.h>
#include <bmcl/Math.h>
#include <bmcl/Logging.h>
#include <bmcl/StringView.h>

#include <cstring>
#include <clocale>

namespace mccgeo {

struct UnitTable {
    bmcl::StringView unitValue;
    double toMeter;
    CoordinateUnits unit;
};

static constexpr std::size_t unitsNum = 21;

static UnitTable unitTable[unitsNum] = {
    {"m", 1.0, CoordinateUnits::Meter},
    {"km", 1000.0, CoordinateUnits::Kilometer},
    {"dm", 1.0/10.0, CoordinateUnits::Decimeter},
    {"cm", 1.0/100.0, CoordinateUnits::Centimeter},
    {"mm", 1.0/1000.0, CoordinateUnits::Millimeter},
    {"kmi", 1852.0, CoordinateUnits::IntNauticalMile},
    {"in", 0.0254, CoordinateUnits::IntInch},
    {"ft", 0.3048, CoordinateUnits::IntFoot},
    {"yd", 0.9144, CoordinateUnits::IntYard},
    {"mi", 1609.344, CoordinateUnits::IntStatuteMile},
    {"fath", 1.8288, CoordinateUnits::IntFathom},
    {"ch", 20.1168, CoordinateUnits::IntChain},
    {"link", 0.201168, CoordinateUnits::IntLink},
    {"us-in", 1.0/39.37, CoordinateUnits::UsSurveyorsInch},
    {"us-ft", 0.304800609601219, CoordinateUnits::UsSurveyorsFoot},
    {"us-yd", 0.914401828803658, CoordinateUnits::UsSurveyorsYard},
    {"us-ch", 20.11684023368047, CoordinateUnits::UsSurveyorsChain},
    {"us-mi", 1609.347218694437, CoordinateUnits::UsSurveyorsStatuteMile},
    {"ind-yd", 0.91439523, CoordinateUnits::IndianYard},
    {"ind-ft", 0.30479841, CoordinateUnits::IndianFoot},
    {"ind-ch", 20.11669506, CoordinateUnits::IndianChain},
};

static CoordinateUnits unitValueToEnum(bmcl::StringView unitValue)
{
    bmcl::ArrayView<UnitTable> view(&unitTable[0], unitsNum);
    for (auto it = view.begin(); it < view.end(); it++) {
        if (it->unitValue == unitValue) {
            return it->unit;
        }
    }
    return CoordinateUnits::None;
}

static inline bool asciiIsSpace(char c)
{
    return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\f';
}

static inline bool asciiIsNotSpace(char c)
{
    return !asciiIsSpace(c);
}

template <typename F>
const char* skipWhile(const char* it, const char* end, F func)
{
    while (true) {
        if (it == end) {
            break;
        }
        if (func(*it)) {
            it++;
        } else {
            break;
        }
    }
    return it;
}

static CoordinateUnits extractUnitsFromDef(bmcl::StringView def, bmcl::StringView unitKey)
{
    const char* it = def.begin();
    const char* end = def.end();
    while (true) {
        it = skipWhile(it, end, asciiIsSpace);
        if (it == end) {
            return CoordinateUnits::None;
        }

        // units=u
        if ((unitKey.size() + 2) > (end - it)) {
            return CoordinateUnits::None;
        }
        if (std::memcmp(unitKey.begin(), it, unitKey.size()) == 0) {
            it += unitKey.size();
            if (*it == '=') {
                it++;
                const char* valueEnd = skipWhile(it, end, asciiIsNotSpace);
                return unitValueToEnum(bmcl::StringView(it, valueEnd));
            }
        }

        it = skipWhile(it, end, asciiIsNotSpace);
    }
}

static const char* _wgs84Definition = "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs";

const char* wgs84Proj4Definition()
{
    return _wgs84Definition;
}

CoordinateConverter::CoordinateConverter(PJ_CONTEXT* ctx, PJ* wgs84Pj)
    : _ctx(ctx)
    , _pj(wgs84Pj)
{
    init();
}

void CoordinateConverter::init()
{
    _hasAngularOutput = proj_angular_output(_pj, PJ_FWD);

    PJ_PROJ_INFO info = proj_pj_info(_pj);

    _def = info.definition;

    bmcl::StringView def(info.definition);
    if (_hasAngularOutput) {
        _units = CoordinateUnits::Degree;
        _vunits = extractUnitsFromDef(def, "vunits");
        if (_vunits == CoordinateUnits::None) {
            _vunits = CoordinateUnits::Meter;
        }
    } else {
        _units = extractUnitsFromDef(def, "units");
        _vunits = extractUnitsFromDef(def, "vunits");
        if (_vunits == CoordinateUnits::None) {
            _vunits = _units;
        }
    }

    _forward.inx = bmcl::degree<double>();
    _forward.iny = bmcl::degree<double>();
    if (_hasAngularOutput) {
        _forward.outx = bmcl::radian<double>();
        _forward.outy = bmcl::radian<double>();
    } else {
        _forward.outx = 1;
        _forward.outy = 1;
    }

    if (_hasAngularOutput) {
        _inverse.inx = bmcl::degree<double>();
        _inverse.iny = bmcl::degree<double>();
    } else {
        _inverse.inx = 1;
        _inverse.iny = 1;
    }
    _inverse.outx = bmcl::radian<double>();
    _inverse.outy = bmcl::radian<double>();
}

CoordinateConverter::~CoordinateConverter()
{
    proj_destroy(_pj);
    proj_context_destroy(_ctx);
}

static std::string lastCtxErrorString(PJ_CONTEXT* ctx)
{
    return std::string(proj_errno_string(proj_context_errno(ctx)));
}

struct ResetLocale {
    ResetLocale()
        : oldLocale(std::setlocale(LC_ALL, NULL))
    {
        std::setlocale(LC_ALL, "C");
    }

    ~ResetLocale()
    {
        std::setlocale(LC_ALL, oldLocale.c_str());
    }

    std::string oldLocale;
};

bmcl::Result<bmcl::Rc<CoordinateConverter>, std::string> CoordinateConverter::createFromProj4Definition(const char* def)
{
    ResetLocale onExit;

    PJ_CONTEXT* ctx = proj_context_create();
    if (!ctx) {
        return std::string("Failed to create PROJ.4 context");
    }

    PJ* pj = proj_create(ctx, def);

    if (!pj) {
        std::string err = lastCtxErrorString(ctx);
        proj_context_destroy(ctx);
        return std::move(err);
    }

    return bmcl::Rc<CoordinateConverter>(new CoordinateConverter(ctx, pj));
}

bmcl::Result<bmcl::Rc<CoordinateConverter>, std::string> CoordinateConverter::create()
{
    return createFromProj4Definition(_wgs84Definition);
}

bmcl::Result<bmcl::NoneType, std::string> CoordinateConverter::initFromProj4Definition(const char* def)
{
    ResetLocale onExit;

    PJ* pj = proj_create(_ctx, def);
    if (!pj) {
        return lastCtxErrorString(_ctx);
    }
    proj_destroy(_pj);
    _pj = pj;

    return bmcl::Result<bmcl::NoneType, std::string>(bmcl::None);
}

Coordinate CoordinateConverter::convert(const Coordinate& coord, const Transform& tr, int direction, PJ* pj)
{
    PJ_COORD out = proj_trans(pj, (PJ_DIRECTION)direction, {{coord.x() * tr.inx, coord.y() * tr.iny, coord.z(), coord.t()}});
    return Coordinate(out.xyzt.x * tr.outx, out.xyzt.y * tr.outy, out.xyzt.z, out.xyzt.t);
}

Coordinate CoordinateConverter::convertForward(const Coordinate& coord) const
{
    return convert(coord, _forward, PJ_FWD, _pj);
}

Coordinate CoordinateConverter::convertInverse(const Coordinate& coord) const
{
    return convert(coord, _inverse, PJ_INV, _pj);
}

bool CoordinateConverter::hasAngularOutput() const
{
    return _hasAngularOutput;
}

CoordinateUnits CoordinateConverter::units() const
{
    return _units;
}

CoordinateUnits CoordinateConverter::vunits() const
{
    return _vunits;
}

const char* CoordinateConverter::definition() const
{
    return _def;
}
}
