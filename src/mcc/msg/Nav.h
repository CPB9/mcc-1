#pragma once
#include "mcc/Config.h"
#include <vector>
#include <bmcl/Option.h>
#include "mcc/msg/Tm.h"
#include "mcc/geo/Position.h"
#include "mcc/geo/Attitude.h"


namespace mccmsg {

class MCC_MSG_DECLSPEC Motion
{
public:
    Motion();
    Motion(const mccgeo::Position& position
        , const mccgeo::Attitude& orientation
        , double accuracy
        , double speed
        , const bmcl::Option<mccgeo::Position>& leadPoint = bmcl::None
        , const bmcl::Option<mccgeo::Position>& velocity = bmcl::None
        , bmcl::Option<double> relativeAltitude = bmcl::None
    );

    mccgeo::Position                position;
    mccgeo::Attitude                orientation;
    bmcl::Option<mccgeo::Position>  leadPoint;
    bmcl::Option<mccgeo::Position>  velocity;
    bmcl::Option<double>            relativeAltitude;
    double                          accuracy;
    double                          speed;
};

class MCC_MSG_DECLSPEC TmMotion : public TmAny
{
public:
    TmMotion(const Device& device, const Motion& s);
    void visit(TmVisitor* visitor) const override;
    const Motion& state() const;
private:
    Motion _state;
};

}
