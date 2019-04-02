#pragma once

#include <QWidget>

#include "mcc/uav/Fwd.h"
#include "mcc/msg/ptr/Protocol.h"
#include "mcc/msg/exts/UavState.h"

namespace mccuav { class UavController; }
namespace mccmav {

class MavlinkToolbarWidget : public QWidget
{
    Q_OBJECT
public:
    MavlinkToolbarWidget(mccuav::UavController* uavController, mccmsg::Protocol protocol);

private slots:
    void uavSelectionChanged(mccuav::Uav* uav);
    void uavTmStorageUpdated(mccuav::Uav* uav);
private:
    void uavStateChanged();
    void sendCmd(bmcl::StringView command, const mccmsg::CmdParams& params);

    mccuav::Uav*     _uav;
    mccuav::UavController* _uavController;
    bmcl::OptionRc<mccmsg::TmUavState> _uavState;

    mccmsg::Protocol _protocol;
    void updateButtonsState();
};

}

