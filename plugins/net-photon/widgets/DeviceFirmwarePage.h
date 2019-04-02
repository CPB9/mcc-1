#pragma once

#include <QObject>
#include <QWidget>
#include <bmcl/Rc.h>
#include <mcc/msg/Objects.h>
#include <mcc/msg/ptr/Tm.h>
#include "mcc/ui/Rc.h"
#include "mcc/uav/Fwd.h"
#include "mcc/ui/Fwd.h"

#include "mcc/calib/CalibrationDialog.h"

class QTabWidget;

namespace photon {
class FirmwareWidget;
class ProjectUpdate;
class NodeView;
class NodeViewUpdater;
} // namespace photon

namespace photongen {
class Validator;
}

namespace mccphoton {

class MccNodeViewModel;
class FirmwareWidget;

class DeviceFirmwarePage {
public:
    DeviceFirmwarePage(mccuav::UavController* uavController, const mccmsg::Device& device, FirmwareWidget* parent, int pageIndex);
    ~DeviceFirmwarePage();
    int pageIndex() const;

    void setProject(const bmcl::Rc<const ::photon::ProjectUpdate>& update);
    void setTmView(const bmcl::Rc<const mccmsg::ITmView>& view);
    void updateTmView(const bmcl::Rc<const mccmsg::ITmViewUpdate>& update);

    QWidget* widget() const;

private:
    mccui::Rc<mccuav::UavController> _uavController;
    mccmsg::Device _device;
    int _pageIndex;

    QTabWidget* _rootWidget;

    ::photon::FirmwareWidget* _ui;
    mcccalib::CalibrationDialog* _calibrationDialog;

    MccNodeViewModel* _nodeViewModel;
    MccNodeViewModel* _eventModel;
    MccNodeViewModel* _statsModel;
    bmcl::Rc<const ::photon::ProjectUpdate> _project;
    bmcl::Rc<photongen::Validator> _validator;

    bmcl::Rc<::photon::NodeView> _statusView;
    bmcl::Rc<::photon::NodeView> _eventView;
    bmcl::Rc<::photon::NodeView> _statsView;

    FirmwareWidget* _parent;
};
}
