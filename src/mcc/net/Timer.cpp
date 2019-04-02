#include "mcc/net/Timer.h"

namespace mccnet {

const bmcl::Option<bmcl::SystemTime>& Timer::started() const
{
    return _started;
}

std::chrono::milliseconds Timer::passed() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::nanoseconds(now() - _time));
}

void Timer::start()
{
    _time = std::chrono::nanoseconds(std::chrono::steady_clock::now().time_since_epoch()).count();
    _started = std::chrono::system_clock::now();
}

void Timer::reset()
{
    _time = 0;
    _started.clear();
}

int64_t Timer::now() const
{
    return std::chrono::nanoseconds(std::chrono::steady_clock::now().time_since_epoch()).count();
}
}
