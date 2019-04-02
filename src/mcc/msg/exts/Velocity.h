#pragma once
#include "mcc/Config.h"
#include "mcc/geo/Position.h"
#include "mcc/msg/exts/ITmExtension.h"
#include <bmcl/Option.h>


namespace mccmsg {

class MCC_MSG_DECLSPEC TmVelocity : public ITmSimpleExtension
{
public:
    TmVelocity(const TmExtensionCounterPtr&);
    ~TmVelocity() override;
    static const TmExtension& id();
    static const char* info();
    bmcl::Option<mccgeo::Position>  velocity() const;
    bmcl::Option<double>            speed() const;
    void set(bmcl::SystemTime t, const bmcl::Option<double>& speed);
    void set(bmcl::SystemTime t, const bmcl::Option<mccgeo::Position>& velocity);
private:
    bmcl::Option<mccgeo::Position>  _velocity;
    bmcl::Option<double>            _speed;
};

}
