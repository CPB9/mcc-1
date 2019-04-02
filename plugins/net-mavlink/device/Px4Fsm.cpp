#include "Px4Fsm.h"

#include "mavlink/standard/mavlink.h"
#include "px4_custom_mode.h"

#include <bmcl/Logging.h>

namespace mccmav {

struct Modes2Name {
    uint8_t     main_mode;
    uint8_t     sub_mode;
    const char* name;       ///< Name for flight mode
    bool        canBeSet;   ///< true: Vehicle can be set to this flight mode
    bool        fixedWing;  /// fixed wing compatible
    bool        multiRotor;  /// multi rotor compatible
};

static const char* manualFlightMode = "Manual";
static const char* altCtlFlightMode = "Altitude";
static const char* posCtlFlightMode = "Position";
static const char* missionFlightMode = "Mission";
static const char* holdFlightMode = "Hold";
static const char* takeoffFlightMode = "Takeoff";
static const char* landingFlightMode = "Land";
static const char* rtlFlightMode = "Return";
static const char* acroFlightMode = "Acro";
static const char* offboardFlightMode = "Offboard";
static const char* stabilizedFlightMode = "Stabilized";
static const char* rattitudeFlightMode = "Rattitude";
static const char* followMeFlightMode = "Follow Me";

static const char* rtgsFlightMode = "Return to Groundstation";

static const char* readyFlightMode = "Ready"; // unused

static const struct Modes2Name rgModes2Name[] = {
    //main_mode                         sub_mode                                name                                      canBeSet  FW      MC
    { PX4_CUSTOM_MAIN_MODE_MANUAL,      0,                                      manualFlightMode,        true,   true,   true },
    { PX4_CUSTOM_MAIN_MODE_STABILIZED,  0,                                      stabilizedFlightMode,    true,   true,   true },
    { PX4_CUSTOM_MAIN_MODE_ACRO,        0,                                      acroFlightMode,          true,   false,  true },
    { PX4_CUSTOM_MAIN_MODE_RATTITUDE,   0,                                      rattitudeFlightMode,     true,   false,  true },
    { PX4_CUSTOM_MAIN_MODE_ALTCTL,      0,                                      altCtlFlightMode,        true,   true,   true },
    { PX4_CUSTOM_MAIN_MODE_POSCTL,      0,                                      posCtlFlightMode,        true,   true,   true },
    { PX4_CUSTOM_MAIN_MODE_AUTO,        PX4_CUSTOM_SUB_MODE_AUTO_LOITER,        holdFlightMode,          true,   true,   true },
    { PX4_CUSTOM_MAIN_MODE_AUTO,        PX4_CUSTOM_SUB_MODE_AUTO_MISSION,       missionFlightMode,       true,   true,   true },
    { PX4_CUSTOM_MAIN_MODE_AUTO,        PX4_CUSTOM_SUB_MODE_AUTO_RTL,           rtlFlightMode,           true,   true,   true },
    { PX4_CUSTOM_MAIN_MODE_AUTO,        PX4_CUSTOM_SUB_MODE_AUTO_FOLLOW_TARGET, followMeFlightMode,      true,   true,   true },
    { PX4_CUSTOM_MAIN_MODE_OFFBOARD,    0,                                      offboardFlightMode,      true,   true,   true },
    // modes that can't be directly set by the user
    { PX4_CUSTOM_MAIN_MODE_AUTO,        PX4_CUSTOM_SUB_MODE_AUTO_LAND,          landingFlightMode,       false,  true,   true },
    { PX4_CUSTOM_MAIN_MODE_AUTO,        PX4_CUSTOM_SUB_MODE_AUTO_READY,         readyFlightMode,         false,  true,   true },
    { PX4_CUSTOM_MAIN_MODE_AUTO,        PX4_CUSTOM_SUB_MODE_AUTO_RTGS,          rtgsFlightMode,          false,  true,   true },
    { PX4_CUSTOM_MAIN_MODE_AUTO,        PX4_CUSTOM_SUB_MODE_AUTO_TAKEOFF,       takeoffFlightMode,       false,  true,   true },
};

DeviceState::DeviceState(uint8_t baseMode, uint32_t customMode, uint8_t systemStatus)
    : _baseMode(baseMode), _customMode(customMode), _systemStatus(systemStatus)
{

}

DeviceState::DeviceState(const DeviceState& other)
{
    _baseMode = other._baseMode;
    _customMode = other._customMode;
    _systemStatus = other._systemStatus;
}

QString DeviceState::toString() const
{
    QString flightMode = "Unknown";

    if (_baseMode & MAV_MODE_FLAG_CUSTOM_MODE_ENABLED) {
        px4_custom_mode px4_mode;
        px4_mode.data = _customMode;

        bool found = false;
        for (size_t i = 0; i < sizeof(rgModes2Name) / sizeof(rgModes2Name[0]); i++) {
            const struct Modes2Name* pModes2Name = &rgModes2Name[i];

            if (pModes2Name->main_mode == px4_mode.main_mode && pModes2Name->sub_mode == px4_mode.sub_mode) {
                flightMode = pModes2Name->name;
                found = true;
                break;
            }
        }

        if (!found) {
            BMCL_WARNING() << "Unknown flight mode" << _customMode;
        }
    }
    else {
        BMCL_WARNING() << "PX4 Flight Stack flight mode without custom mode enabled?";
    }

    return flightMode;
}

bmcl::Option<DeviceState> DeviceState::fromString(const QString& name)
{
    for (size_t i = 0; i < sizeof(rgModes2Name) / sizeof(rgModes2Name[0]); i++) {
        const struct Modes2Name* pModes2Name = &rgModes2Name[i];

        if (name.compare(pModes2Name->name, Qt::CaseInsensitive) == 0) {
            union px4_custom_mode px4_mode;

            px4_mode.data = 0;
            px4_mode.main_mode = pModes2Name->main_mode;
            px4_mode.sub_mode = pModes2Name->sub_mode;

            uint8_t baseMode = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
            uint32_t customMode = px4_mode.data;
            
            return DeviceState(baseMode, customMode, 0);
        }
    }

    return bmcl::None;
}

bmcl::Option<size_t> DeviceState::toUint() const
{
    if (_baseMode & MAV_MODE_FLAG_CUSTOM_MODE_ENABLED) {
        px4_custom_mode px4_mode;
        px4_mode.data = _customMode;

        for (size_t i = 0; i < sizeof(rgModes2Name) / sizeof(rgModes2Name[0]); i++) {
            const struct Modes2Name* pModes2Name = &rgModes2Name[i];

            if (pModes2Name->main_mode == px4_mode.main_mode && pModes2Name->sub_mode == px4_mode.sub_mode) {
                return i;
            }
        }
    }
    return bmcl::None;
}

bool DeviceState::isArmed() const
{
    return _baseMode & MAV_MODE_FLAG_DECODE_POSITION_SAFETY;
}

bool DeviceState::isEmergency() const
{
    return (_systemStatus == MAV_STATE::MAV_STATE_EMERGENCY);
}

uint8_t DeviceState::baseMode() const
{
    return _baseMode;
}

uint32_t DeviceState::customMode() const
{
    return _customMode;
}

uint8_t DeviceState::systemStatus() const
{
    return _systemStatus;
}

}

