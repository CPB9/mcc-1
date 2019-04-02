#include "mcc/msg/Nav.h"


namespace mccmsg {

Motion::Motion() : Motion(mccgeo::Position(), mccgeo::Attitude(), 0.0, 0.0) {}
Motion::Motion(const mccgeo::Position& position
    , const mccgeo::Attitude& orientation
    , double accuracy
    , double speed
    , const bmcl::Option<mccgeo::Position>& leadPoint
    , const bmcl::Option<mccgeo::Position>& velocity
    , bmcl::Option<double> relativeAltitude
)
    : position(position)
    , orientation(orientation)
    , leadPoint(leadPoint)
    , velocity(velocity)
    , relativeAltitude(relativeAltitude)
    , accuracy(accuracy)
    , speed(speed)
{
}

TmMotion::TmMotion(const Device& device, const Motion& s) : TmAny(device), _state(s) {}
const Motion& TmMotion::state() const { return _state; }

}
