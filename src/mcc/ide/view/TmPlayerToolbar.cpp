#include "mcc/ide/view/TmPlayerToolbar.h"

#include <QToolBar>
#include <QAction>
#include <QSlider>
#include <QComboBox>

#include "mcc/uav/UavController.h"
#include "mcc/uav/ExchangeService.h"

namespace mccide {

TmPlayerToolbar::TmPlayerToolbar(mccuav::UavController* uavController, mccuav::ExchangeService* service)
    : _manager(uavController)
    , _service(service)
{
    setObjectName("TmPlayerToolbar");

    _devicesComboBox = new QComboBox(this);
    _devicesComboBox->setFixedWidth(150);

    addWidget(_devicesComboBox);

    _playAction    = addAction(QIcon(":/player/play.png"), "Play");
    _stopAction    = addAction(QIcon(":/player/stop.png"), "Stop");
    _rewindAction  = addAction(QIcon(":/player/rewind.png"), "Rewind");
    _forwardAction = addAction(QIcon(":/player/forward.png"), "Forward");

    _slider = new QSlider(Qt::Horizontal, this);
    addWidget(_slider);

    connect(uavController, &mccuav::UavController::uavAdded, this,
        [this](mccui::FlyingDevice* dev)
        {
            _devicesComboBox->addItem(QString::fromStdString(dev->deviceDescription()._device_info), dev->name());
        });

    connect(uavController, &mccuav::UavController::uavRemoved, this, [this](mccui::FlyingDevice* dev) {});

    connect(uavController, &mccuav::UavController::uavFirmwareLoaded, this,
        [this](mccui::FlyingDevice* dev)
        {
            auto index = _devicesComboBox->findData(dev->name());
            _devicesComboBox->setItemText(index, QString::fromStdString(dev->deviceDescription()._device_info));
        });

    connect(service, &mccuav::ExchangeService::deviceActionList, this,
        [](mcc::messages::Device device, const mcc::messages::DeviceActions& actions)
        {

        });

    connect(_devicesComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
        [this](int index)
        {
            emit _service->onDeviceActionList(misc::Uuid(_devicesComboBox->itemData(index).toString()));
        }
    );
}
}
