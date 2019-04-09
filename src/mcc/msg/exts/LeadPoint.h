#pragma once
#include "mcc/Config.h"
#include "mcc/geo/Position.h"
#include "mcc/msg/exts/ITmExtension.h"
#include <bmcl/Option.h>


namespace mccmsg {

class MCC_MSG_DECLSPEC TmLeadPoint : public ITmSimpleExtension
{
public:
    TmLeadPoint(const TmExtensionCounterPtr&);
    ~TmLeadPoint() override;
    static const TmExtension& id();
    static const char* info();
    const bmcl::Option<mccgeo::Position>& position() const;
    void set(bmcl::SystemTime t, const bmcl::Option<mccgeo::Position>&);
private:
    bmcl::Option<mccgeo::Position> _leadPoint;
};

}
