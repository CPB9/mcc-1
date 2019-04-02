#include "mcc/uav/Structs.h"

namespace mccuav {

TrackSettings::TrackSettings()
    : _show(true)
    , _mode(TrackMode::All)
    , _seconds(100)
    , _meters(10000)
{
}

TrackMode TrackSettings::mode() const
{
    if (_show)
        return _mode;
    else
        return TrackMode::None;
}

void TrackSettings::setMode(TrackMode mode)
{
    if (mode == TrackMode::None)
        _show = false;
    else
    {
        _show = true;
        _mode = mode;
    }
}

int TrackSettings::value() const
{
    if (_mode == TrackMode::Distance)
        return _meters;
    else if (_mode == TrackMode::Time)
        return _seconds;

    return 0;
}

void TrackSettings::setValue(int value)
{
    if (_mode == TrackMode::Distance)
        _meters = value;
    else if (_mode == TrackMode::Time)
        _seconds = value;
}

TrackMode TrackSettings::lastMode() const { return _mode; }
int TrackSettings::seconds() const { return _seconds; }
int TrackSettings::meters() const { return _meters; }
void TrackSettings::setSeconds(int seconds) { _seconds = seconds; }
void TrackSettings::setMeters(int meters) { _meters = meters; }

}
