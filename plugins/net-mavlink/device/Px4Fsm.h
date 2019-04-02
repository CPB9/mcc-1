#pragma once

#include <cstdint>
#include <QString>

#include <bmcl/Option.h>

namespace mccmav {

struct DeviceState
{
    DeviceState(uint8_t baseMode, uint32_t customMode, uint8_t systemStatus);
    DeviceState(const DeviceState& other);
    QString toString() const;
    static bmcl::Option<DeviceState> fromString(const QString& name);
    bmcl::Option<size_t> toUint() const;
    bool isArmed() const;
    bool isEmergency() const;

    uint8_t baseMode() const;
    uint32_t customMode() const;
    uint8_t systemStatus() const;
private:
    uint8_t _baseMode;
    uint32_t _customMode;
    uint8_t _systemStatus;
};

}