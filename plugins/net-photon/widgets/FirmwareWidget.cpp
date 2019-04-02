#include "../Firmware.h"
#include "../widgets/FirmwareWidget.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/ChannelsController.h"
#include "mcc/msg/TmView.h"

#include <bmcl/SharedBytes.h>

#include <photon/model/CmdModel.h>
#include <photon/model/ValueInfoCache.h>
#include <photon/ui/FirmwareWidget.h>
#include <photon/ui/QNodeModel.h>
#include <photon/ui/QNodeViewModel.h>
#include <photon/groundcontrol/ProjectUpdate.h>
#include "../widgets/MccNodeViewModel.h"
#include "../widgets/DeviceFirmwarePage.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedLayout>

namespace mccphoton {

FirmwareWidget::FirmwareWidget(const mccmsg::Protocol& protocol, mccuav::UavController* uavController)
    : _uavController(uavController)
    , _protocol(protocol)
{
    using mccuav::UavController;

    _groupCmdButton = new QPushButton("Команда группе");
    _groupCmdButton->setCheckable(true);

    QHBoxLayout* groupCmdLayout = new QHBoxLayout();
    groupCmdLayout->addStretch();
    groupCmdLayout->addWidget(_groupCmdButton);
    groupCmdLayout->setContentsMargins(0, 0, 9, 9);
    QVBoxLayout* mainLayout = new QVBoxLayout();

    _layout = new QStackedLayout();
    mainLayout->addLayout(_layout);
    mainLayout->addLayout(groupCmdLayout);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    auto manager = _uavController.get();
    connect(manager, &UavController::selectionChanged, this, &FirmwareWidget::selectionChanged);
    connect(manager, &UavController::uavFirmwareLoaded, this, &FirmwareWidget::firmwareLoaded);
    connect(manager, &UavController::uavStateChanged, this, &FirmwareWidget::deviceStateChanged);
    connect(manager, &UavController::setTmView, this, &FirmwareWidget::setTmView);
    connect(manager, &UavController::updateTmStatusView, this, &FirmwareWidget::updateTmStatusView);
}

FirmwareWidget::~FirmwareWidget()
{
}

bool FirmwareWidget::isGroupTarget() const
{
    return _groupCmdButton->isChecked();
}

void FirmwareWidget::selectionChanged(mccuav::Uav* device)
{
    if (!device)
    {
        //TODO: set empty device page
        return;
    }

    if (device->protocol() != _protocol)
        return;

    if (device->firmwareDescription().isNone())
        return;

    auto frm = bmcl::dynamic_pointer_cast<const Firmware>(device->firmwareDescription()->frm());
    if (frm.isNull())
        return;
    setProject(device, frm->pkg());

    auto it = _devicePages.find(device->device());
    if (it == _devicePages.end())
    {
        BMCL_CRITICAL() << "Can't find device page!";
        return;
    }
    _layout->setCurrentIndex(it->second->pageIndex());
}

void FirmwareWidget::firmwareLoaded(mccuav::Uav* uav)
{
    selectionChanged(uav);
}

void FirmwareWidget::deviceStateChanged(mccuav::Uav* uav)
{
    if (_uavController->selectedUav() != uav)
        return;

    bool isGroup = uav->group().isSome();
    if (_groupCmdButton->isVisible() != isGroup)
        _groupCmdButton->setVisible(isGroup);
}

void FirmwareWidget::setProject(mccuav::Uav* uav, const ::photon::ProjectUpdate::ConstPointer& update)
{
    auto it = _devicePages.find(uav->device());
    if (it == _devicePages.end())
    {
        auto fwPage = std::make_unique<DeviceFirmwarePage>(_uavController.get(), uav->device(), this, _layout->count());
        _layout->addWidget(fwPage->widget());
        it = _devicePages.emplace(uav->device(), std::move(fwPage)).first;
    }

    forwardToFirmwarePage(uav->device(), &DeviceFirmwarePage::setProject, update);
    _groupCmdButton->setVisible(uav->group().isSome());
}

void FirmwareWidget::setTmView(const bmcl::Rc<const mccmsg::ITmView>& view)
{
    forwardToFirmwarePage(view->device(), &DeviceFirmwarePage::setTmView, view);
}

void FirmwareWidget::updateTmStatusView(const bmcl::Rc<const mccmsg::ITmViewUpdate>& update)
{
    forwardToFirmwarePage(update->device(), &DeviceFirmwarePage::updateTmView, update);
}

template <typename F, typename... A>
void FirmwareWidget::forwardToFirmwarePage(const mccmsg::Device& id, F&& func, A&&... args)
{
    auto it = _devicePages.find(id);
    if (it == _devicePages.end())
    {
        return;
    }

    ((it->second.get())->*func)(std::forward<A>(args)...);
}
}
