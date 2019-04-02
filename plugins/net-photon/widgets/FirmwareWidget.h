#pragma once

#include <QWidget>

#include "mcc/msg/ptr/Fwd.h"
#include "mcc/msg/Objects.h"
#include "mcc/ui/Rc.h"
#include "mcc/uav/Fwd.h"
#include "mcc/ui/Fwd.h"

#include <photon/ui/FirmwareWidget.h>
#include <photon/groundcontrol/ProjectUpdate.h>
#include <photongen/groundcontrol/Validator.hpp>
#include <decode/parser/Project.h>
#include <photon/model/NodeView.h>

#include <memory>

class QStackedLayout;
class QPushButton;

namespace decode { struct ProjectUpdate; }

namespace mccphoton {

class DeviceFirmwarePage;

struct DeviceTmViews {

};

class FirmwareWidget : public QWidget
{
    Q_OBJECT

public:
    FirmwareWidget(const mccmsg::Protocol& protocol, mccuav::UavController* uavController);
    ~FirmwareWidget();

    bool isGroupTarget() const;

private slots:
    void selectionChanged(mccuav::Uav* uav);
    void firmwareLoaded(mccuav::Uav* uav);
    void deviceStateChanged(mccuav::Uav* uav);

    void setProject(mccuav::Uav* uav, const ::photon::ProjectUpdate::ConstPointer& proj);
    void setTmView(const bmcl::Rc<const mccmsg::ITmView>& view);
    void updateTmStatusView(const bmcl::Rc<const mccmsg::ITmViewUpdate>& update);

private:
    template <typename F, typename... A>
    void forwardToFirmwarePage(const mccmsg::Device& device, F&& func, A&&... args);
private:
    mccmsg::Protocol _protocol;

    QStackedLayout * _layout;
    QPushButton* _groupCmdButton;

    std::map<mccmsg::Device, std::unique_ptr<DeviceFirmwarePage>> _devicePages;
    std::map<mccmsg::Device, DeviceTmViews> _tmViews;
    mccui::Rc<mccuav::UavController> _uavController;
};
}
