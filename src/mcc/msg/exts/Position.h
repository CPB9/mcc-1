#pragma once
#include "mcc/Config.h"
#include "mcc/geo/Position.h"
#include "mcc/msg/exts/ITmExtension.h"
#include <bmcl/Option.h>


namespace mccmsg {

class MCC_MSG_DECLSPEC TmPosition : public ITmSimpleExtension
{
public:
    TmPosition(const TmExtensionCounterPtr&);
    ~TmPosition() override;
    static const TmExtension& id();
    static const char* info();
    const bmcl::Option<mccgeo::Position>& position() const;
    const bmcl::Option<double>& positionAccuracy() const;
    void set(bmcl::SystemTime t, const bmcl::Option<mccgeo::Position>& v, bmcl::Option<double> acc = bmcl::None);
private:
    bmcl::Option<double> _positionAccuracy;
    bmcl::Option<mccgeo::Position> _position;
};

}
