#pragma once
#include "mcc/Config.h"
#include "mcc/geo/Attitude.h"
#include "mcc/msg/exts/ITmExtension.h"
#include <bmcl/Option.h>


namespace mccmsg {

class MCC_MSG_DECLSPEC TmAttitude : public ITmSimpleExtension
{
public:
    TmAttitude(const TmExtensionCounterPtr&);
    ~TmAttitude() override;
    static const TmExtension& id();
    static const char* info();
    bmcl::Option<mccgeo::Attitude> attitude() const;
    void set(bmcl::SystemTime t, const bmcl::Option<mccgeo::Attitude>& v);
private:
    bmcl::Option<mccgeo::Attitude> _attitude;
};

}
