#pragma once
#include "mcc/Config.h"
#include "mcc/msg/exts/ITmExtension.h"
#include <bmcl/Option.h>
#include <vector>

namespace mccmsg {

enum class GpsFixType
{
    NoGps,      //No GPS connected
    NoFix,      // No solution
    Fix2D,      // 2D Fix
    Fix3D,      // 3D Fix
    DGps,       // DGPS/SBAS aided 3D position
    Single,    // Single point position
    PsrDiff,    // Pseudorange differential solution
    L1Float,    // Floating L1 ambiguity solution
    L1Int,      // Integer L1 ambiguity solution
    Static,     // Static fixed, typically used for base stations
};

MCC_MSG_DECLSPEC const char* toShortStr(GpsFixType t);
MCC_MSG_DECLSPEC const char* toString(GpsFixType t);

struct MCC_MSG_DECLSPEC GpsSat
{
    GpsSat(uint8_t id, uint8_t elevation, uint8_t azimuth, uint8_t snr, bool isUsed);
    bool operator==(const GpsSat& other) const;
    bool operator!=(const GpsSat& other) const;
    uint8_t id;          //Global satellite ID
    uint8_t elevation;    //Elevation (0: right on top of receiver, 90: on the horizon) of satellite
    uint8_t azimuth;      //Direction of satellite, 0: 0 deg, 255: 360 deg.
    uint8_t snr;          //Signal to noise ratio of satellite
    bool    isUsed;         //0: Satellite not used, 1: used for localization
};
using GpsSats = std::vector<GpsSat>;

class MCC_MSG_DECLSPEC TmGps : public ITmSimpleExtension
{
public:
    TmGps(const TmExtensionCounterPtr&);
    ~TmGps() override;
    static const TmExtension& id();
    static const char* info();
    const GpsSats& satellites() const;
    uint8_t count() const;
    bmcl::Option<GpsFixType> fixType() const;

    void set(bmcl::SystemTime t, GpsSats&& sats);
    void set(bmcl::SystemTime t, uint8_t count, bmcl::Option<GpsFixType>);
private:
    GpsSats _sats;
    uint8_t _count;
    bmcl::Option<GpsFixType>  _fixType;
};

}
