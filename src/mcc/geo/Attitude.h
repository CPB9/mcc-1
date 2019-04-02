#pragma once

#include "mcc/geo/Config.h"

namespace mccgeo {

class MCC_GEO_DECLSPEC Attitude {
public:
    inline Attitude() : _heading(0.0) , _pitch(0.0) , _roll(0.0) { }
    inline Attitude(double heading, double pitch, double roll) : _heading(heading) , _pitch(pitch) , _roll(roll) { }
    inline double heading() const { return _heading; }
    inline double pitch() const { return _pitch;}
    inline double roll() const { return _roll; }
    inline double& heading() { return _heading; }
    inline double& pitch() { return _pitch; }
    inline double& roll() { return _roll; }
    inline void setHeading(double heading) { _heading = heading; }
    inline void setPitch(double pitch) { _pitch = pitch; }
    inline void setRoll(double roll) { _roll = roll; }
    bool operator==(const Attitude& other) const;
    bool operator!=(const Attitude& other) const;
private:
    double _heading;
    double _pitch;
    double _roll;
};


}
