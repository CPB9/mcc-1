#include "../widgets/DeviceFirmwarePage.h"
#include "../widgets/MccNodeViewModel.h"
#include "../Firmware.h"
#include "../widgets/FirmwareWidget.h"

#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/UavController.h"

#include "mcc/msg/Cmd.h"
#include "mcc/msg/TmView.h"

#include <bmcl/SharedBytes.h>

#include <photon/groundcontrol/ProjectUpdate.h>
#include <photon/model/CmdModel.h>
#include <photon/model/ValueInfoCache.h>
#include <photon/ui/FirmwareWidget.h>
#include <photon/ui/QNodeModel.h>
#include <photon/ui/QNodeViewModel.h>

#include <QHBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QProgressBar>

namespace mccphoton {

DeviceFirmwarePage::~DeviceFirmwarePage() {}
DeviceFirmwarePage::DeviceFirmwarePage(mccuav::UavController* uavController, const mccmsg::Device& device, FirmwareWidget* parent, int pageIndex)
    : _uavController(uavController)
    , _device(device)
    , _pageIndex(pageIndex)
    , _parent(parent)
{
    bmcl::Rc<::photon::Node> emptyNode = new ::photon::Node(bmcl::None);

    auto paramViewModel = std::make_unique<MccNodeViewModel>(new ::photon::NodeView(emptyNode.get()));
    _nodeViewModel = paramViewModel.get();
    _nodeViewModel->setDevice(_device);

    auto eventViewModel = std::make_unique<MccNodeViewModel>(new ::photon::NodeView(emptyNode.get()));
    _eventModel = eventViewModel.get();

    auto statsViewModel = std::make_unique<MccNodeViewModel>(new ::photon::NodeView(emptyNode.get()));
    _statsModel = statsViewModel.get();

    _ui = new ::photon::FirmwareWidget(std::move(paramViewModel), std::move(eventViewModel), std::move(statsViewModel));

    auto deviceGroup = [this](const mccmsg::Device& device) -> bmcl::Option<mccmsg::Group> {
        auto dm = _uavController.get();
        auto uav = dm->uav(_device);
        if (uav.isNone()) {
            assert(false);
            return bmcl::None;
        }
        if (!_parent->isGroupTarget())
            return bmcl::None;

        return uav->group();
    };

    QObject::connect(_ui, &::photon::FirmwareWidget::unreliablePacketQueued,
                     [this, deviceGroup](const ::photon::PacketRequest& packet) {
                         auto grp = deviceGroup(_device);

                         if (grp.isSome())
                         {
                             _uavController->sendCmdYY(new CmdPacketPhoton(grp.unwrap(), _device, packet, false));
                         }
                         else
                         {
                             _uavController->sendCmdYY(new CmdPacketPhoton(_device, packet, false));
                         }
                     });

    QObject::connect(_ui, &::photon::FirmwareWidget::reliablePacketQueued,
                     [this, deviceGroup](const ::photon::PacketRequest& packet) {
                         BMCL_DEBUG() << "send to: " << _device.toStdString();

                         auto grp = deviceGroup(_device);

                         if (grp.isSome())
                         {
                             _uavController->sendCmdYY(new CmdPacketPhoton(grp.unwrap(), _device, packet, true));
                         }
                         else
                         {
                             _uavController->sendCmdYY(new CmdPacketPhoton(_device, packet, true));
                         }
                     });

    QObject::connect(_uavController.get(), &mccuav::UavController::tmPaketResponse,
                     [this](const bmcl::Rc<const mccmsg::ITmPacketResponse>& response) {
                         if (_device != response->device())
                             return;
                         bmcl::Rc<const TmPacketResponsePhoton> r = bmcl::dynamic_pointer_cast<const TmPacketResponsePhoton>(response);
                         _ui->acceptPacketResponse(r->response());
                     });

    QObject::connect(_uavController.get(), &mccuav::UavController::traitCalibration,
                     [this](const mccmsg::TmCalibrationPtr& msg)
                     {
                         _calibrationDialog->onTraitCalibration(msg);
                     }
    );
    QObject::connect(_uavController.get(), &mccuav::UavController::traitCommonCalibrationStatus,
                     [this](const mccmsg::TmCommonCalibrationStatusPtr& msg)
                     {
                         _calibrationDialog->onCommonCalibrationStatus(msg);
                     }
    );

    _rootWidget = new QTabWidget();
    _rootWidget->addTab(_ui, "Конфигуратор");
    _calibrationDialog = new mcccalib::CalibrationDialog(uavController, 0, _device);
    _rootWidget->addTab(_calibrationDialog, "Калибровка");

    QObject::connect(_uavController.get(), &mccuav::UavController::traitCalibration,
            [this](const mccmsg::TmCalibrationPtr& msg)
            {
                _calibrationDialog->onTraitCalibration(msg);
            }
    );
    QObject::connect(_uavController.get(), &mccuav::UavController::traitCommonCalibrationStatus,
            [this](const mccmsg::TmCommonCalibrationStatusPtr& msg)
            {
                _calibrationDialog->onCommonCalibrationStatus(msg);
            }
    );
}

int DeviceFirmwarePage::pageIndex() const
{
    return _pageIndex;
}

void DeviceFirmwarePage::setProject(const bmcl::Rc<const ::photon::ProjectUpdate>& update)
{
    if (_project == update)
        return;

    _project = update;

    if (bmcl::anyNull(_eventView, _statusView, _statsView))
    {
        BMCL_CRITICAL() << "EventView or statusView is not set! Project will not applied!";
        return;
    }

    _validator.reset(new photongen::Validator(update->project(), update->device()));
    _ui->setValidator(_validator);

    bmcl::Rc<::photon::CmdModel> cmdNode = new ::photon::CmdModel(update->device(), update->cache(), bmcl::None);
    _ui->setRootCmdNode(update->cache(), cmdNode.get());
    _ui->setRootTmNode(_statusView.get(), _eventView.get(), _statsView.get());
}
void DeviceFirmwarePage::setTmView(const bmcl::Rc<const mccmsg::ITmView>& view)
{
    bmcl::Rc<const TmView> photonView = bmcl::dynamic_pointer_cast<const TmView>(view);
    if (photonView.isNull())
    {
        assert(false);
        return;
    }
    _statusView = photonView->statusView();
    _eventView = photonView->eventView();
    _statsView = photonView->statsView();

    _ui->setRootTmNode(_statusView.get(), _eventView.get(), _statsView.get());
}

class Visitor : public ITmUpdateVisitor
{
public:
    Visitor(::photon::FirmwareWidget* ui) : _ui(ui) {}
    ~Visitor() override {}
    void visit(const TmUpdateSES* v) override { _ui->applyTmUpdates(v->statusUpdate().get(), v->eventUpdate().get(), v->statsUpdate().get()); }
    void visit(const TmUpdateSub* v) override {}
private:
    ::photon::FirmwareWidget* _ui;
};
void DeviceFirmwarePage::updateTmView(const bmcl::Rc<const mccmsg::ITmViewUpdate>& update)
{
    bmcl::Rc<const TmViewUpdatePhoton> photonUpdate = bmcl::dynamic_pointer_cast<const TmViewUpdatePhoton>(update);
    if (photonUpdate.isNull())
    {
        assert(false);
        return;
    }
    //downcast
    Visitor v(_ui);
    photonUpdate->visit(v);
}

QWidget* DeviceFirmwarePage::widget() const
{
    return _rootWidget;
}
}
