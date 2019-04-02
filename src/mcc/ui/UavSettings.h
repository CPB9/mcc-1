#pragma once
#include "mcc/Config.h"

namespace mccui {

class UavSettings
{
public:
    inline UavSettings() { _exchangeWithoutRegistration = false; }

    inline UavSettings(bool exchangeWithoutRegistration)
        : _exchangeWithoutRegistration(exchangeWithoutRegistration)
    {
    }

    inline bool exchangeWithoutRegistration() const { return _exchangeWithoutRegistration; }
private:
    bool _exchangeWithoutRegistration;
};

}
