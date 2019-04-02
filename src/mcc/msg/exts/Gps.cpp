#include "mcc/msg/exts/Gps.h"
#include <bmcl/Option.h>

namespace mccmsg {

GpsSat::GpsSat(uint8_t id, uint8_t elevation, uint8_t azimuth, uint8_t snr, bool isUsed)
    : id(id), elevation(elevation), azimuth(azimuth), snr(snr), isUsed(isUsed)
{
}

bool GpsSat::operator==(const GpsSat& other) const { return id == other.id && elevation == other.elevation && azimuth == other.azimuth && snr == other.snr && isUsed == other.isUsed; }
bool GpsSat::operator!=(const GpsSat& other) const { return !(*this == other); }

TmGps::TmGps(const TmExtensionCounterPtr& counter) : ITmSimpleExtension(id(), info(), counter) {}
TmGps::~TmGps() {}
const TmExtension& TmGps::id() { static auto i = TmExtension::createOrNil("{04e1c137-8ed9-4da5-aaa8-7bf6a8fede4e}"); return i; }
const char* TmGps::info() { return "gps"; }

const GpsSats& TmGps::satellites() const { return _sats; }
uint8_t TmGps::count() const { return _count; }
bmcl::Option<GpsFixType> TmGps::fixType() const { return _fixType; }

void TmGps::set(bmcl::SystemTime t, GpsSats&& sats)
{
    bool changed = (sats != _sats);
    if (changed)
        _sats = std::move(sats);
    updated_(t, changed);
}
void TmGps::set(bmcl::SystemTime t, uint8_t count, bmcl::Option<GpsFixType> fixType)
{
    bool changed = (_count != count || fixType != _fixType);
    _count = count;
    _fixType = fixType;
    updated_(t, changed);
}

const char* toShortStr(GpsFixType t)
{
    switch (t)
    {
    case GpsFixType::NoGps: return "NoGps";      //No GPS connected
    case GpsFixType::NoFix: return "NoFix";      // No solution
    case GpsFixType::Fix2D: return "Fix2D";      // 2D Fix
    case GpsFixType::Fix3D: return "Fix3D";      // 3D Fix
    case GpsFixType::DGps: return "DGps";       // DGPS/SBAS aided 3D position
    case GpsFixType::Single: return "Single";    // Single point position
    case GpsFixType::PsrDiff: return "PsrDiff";    // Pseudorange differential solution
    case GpsFixType::L1Float: return "L1Float";    // Floating L1 ambiguity solution
    case GpsFixType::L1Int: return "L1Int";      // Integer L1 ambiguity solution
    case GpsFixType::Static: return "Static";     // Static fixed, typically used for base stations
    default:
        break;
    }
    return "-";
}

const char* toString(GpsFixType t)
{
    switch (t)
    {
    case GpsFixType::NoGps: return "No GPS connected";
    case GpsFixType::NoFix: return "No solution";
    case GpsFixType::Fix2D: return "2D Fix";
    case GpsFixType::Fix3D: return "3D Fix";
    case GpsFixType::DGps: return "DGPS/SBAS aided 3D position";
    case GpsFixType::Single: return "Single point position";
    case GpsFixType::PsrDiff: return "Pseudorange differential solution";
    case GpsFixType::L1Float: return "Floating L1 ambiguity solution";
    case GpsFixType::L1Int: return "Integer L1 ambiguity solution";
    case GpsFixType::Static: return "Static fixed, typically used for base stations";
    default:
        break;
    }
    return "-";
}


}
