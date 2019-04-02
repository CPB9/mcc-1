#pragma once

#include "mcc/Config.h"
#include "mcc/uav/Rc.h"
#include "mcc/ui/Trackable.h"

namespace mccuav {

class UavController;
class Uav;

class MCC_UAV_DECLSPEC UavTrackable : public mccui::Trackable
{
    Q_OBJECT
public:
    UavTrackable(mccuav::UavController* uavController);
    ~UavTrackable() override;
    bmcl::Option<mccgeo::LatLon> position() const override;

private:
    void connectUav(mccuav::Uav* uav);
    void disconnectUav(mccuav::Uav* uav);

    mccuav::Rc<mccuav::UavController>   _manager;
    mccuav::Uav*                        _currentUav;
};
}
