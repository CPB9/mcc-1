#pragma once

#include "mcc/Config.h"

#include <bmcl/Option.h>
#include <bmcl/TimeUtils.h>

#include <chrono>

namespace mccnet {

class MCC_PLUGIN_NET_DECLSPEC Timer
{
public:
    inline Timer() = default;
    const bmcl::Option<bmcl::SystemTime>& started() const;
    std::chrono::milliseconds passed() const;
    void start();
    void reset();
private:
    int64_t now() const;
    int64_t _time;
    bmcl::Option<bmcl::SystemTime> _started;
};
}

