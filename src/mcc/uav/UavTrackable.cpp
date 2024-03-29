#include "mcc/uav/UavTrackable.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/Uav.h"

namespace mccuav {

UavTrackable::~UavTrackable(){}

UavTrackable::UavTrackable(mccuav::UavController* uavController)
    : _manager(uavController)
{
    _currentUav = _manager->selectedUav();
    connectUav(_currentUav);
    connect(_manager.get(), &mccuav::UavController::selectionChanged, this, [this](mccuav::Uav* device) {
        if (device) {
            emit trackingEnabled(true);
        }
        else {
            emit trackingEnabled(false);
        }
        disconnectUav(_currentUav);
        _currentUav = device;
        connectUav(_currentUav);
    });

    connect(_manager.get(), &mccuav::UavController::uavRemoved, this, [this](mccuav::Uav* device) {
        if (device == _currentUav) {
            disconnectUav(device);
            emit trackingStopped();
            _currentUav = nullptr;
        }
        if (_manager->uavsList().size() == 0) {
            emit trackingEnabled(false);
        }
    });
}

void UavTrackable::connectUav(mccuav::Uav* uav)
{
    if (!uav) {
        return;
    }
    connect(uav, &mccuav::Uav::positionChanged, this, [uav, this]() {
        auto pos = uav->position();
        if(pos.isSome())
            emit positionUpdated(pos->latLon());
    });

    connect(uav, &mccuav::Uav::signalBad, this, &UavTrackable::trackingStopped);
}

void UavTrackable::disconnectUav(mccuav::Uav* uav)
{
    if (!uav) {
        return;
    }
    disconnect(uav, &mccuav::Uav::positionChanged, this, nullptr);
    disconnect(uav, &mccuav::Uav::signalBad, this, nullptr);
    emit trackingStopped();
}

bmcl::Option<mccgeo::LatLon> UavTrackable::position() const
{
    if(_currentUav && _currentUav->position().isSome())
        return _currentUav->position()->latLon();

    return bmcl::None;
}
}
