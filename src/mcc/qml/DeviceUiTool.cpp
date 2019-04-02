#include "mcc/qml/DeviceUiTool.h"

#include <QDir>
#include <QTemporaryDir>
#include <QStackedLayout>
#include <QUrl>
#include <QLabel>
#include <QMessageBox>
#include <QApplication>

#include "bmcl/Logging.h"

#include "mcc/qml/DeviceUiWidget.h"

#include "mcc/ide/view/ZipExtractor.h"

#include "mcc/uav/PointOfInterest.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/UavUiController.h"
#include "mcc/uav/GroupsController.h"

#include "mcc/map/drawables/Flag.h"

#include "mcc/msg/Cmd.h"
#include "mcc/msg/ParamList.h"
#include "mcc/msg/Error.h"

#include <QIcon>

namespace mccqml {

DeviceUiTool::DeviceUiTool(mccui::UserNotifier* userNotifier, mccuav::UavController* uavController, mccuav::UavUiController* uiController, mccuav::GroupsController* groupsController, QWidget* parent)
    : QWidget(parent)
    , _userNotifier(userNotifier)
    , _uavController(uavController)
    , _uiController(uiController)
    , _groupsController(groupsController)
{
    _uiDir = new QTemporaryDir;
    setObjectName("DeviceUiTool");
    setWindowTitle("Интерфейс аппарата");
    setWindowIcon(QIcon(":/qml/icon.png"));

    _layout = new QStackedLayout();
    _layout->addWidget(new QLabel("Не выбран аппарат / не загружен пользовательский интерфейс для аппарата"));

    setLayout(_layout);
    connect(_uavController, &mccuav::UavController::selectionChanged, this, &DeviceUiTool::onDeviceChanged);
    connect(_uiController, &mccuav::UavUiController::openUi, this, &DeviceUiTool::deviceUiUpdated);
    connect(_uavController, &mccuav::UavController::uavRemoved, this, &DeviceUiTool::onDeviceRemoved);
}

DeviceUiTool::~DeviceUiTool()
{
    delete _uiDir;
}

void DeviceUiTool::deviceUiUpdated(mccmsg::Device name, const bmcl::OptionRc<mccuav::UavUi>& ui)
{
    auto device = _uavController->uav(name);
    if (device.isNone())
    {
        assert(false);
        return;
    }
    if (_indexMap.contains(*device))
    {
        int index = _indexMap[*device];
        _indexMap.remove(*device);

        for (auto key : _indexMap.keys())
        {
            if (_indexMap[key] >= index)
                _indexMap[key]--;
        }

        auto widget = _layout->takeAt(index)->widget();
        _layout->removeWidget(widget);
        delete widget;
    }

    if (ui.isNone())
        return;

    auto deviceExtension = new DeviceUiWidget(_userNotifier, _groupsController, _uavController, _uiController, this);
    deviceExtension->setDevice(device->device());
    deviceExtension->setEnableSwitchLocalOnboard(true);
    if (!deviceExtension->load(QUrl::fromLocalFile(ui->originPath())))
    {
        BMCL_DEBUG() << "Ошибка при создании окна QML: " << deviceExtension->errorString().toStdString();
        //QMessageBox::warning(this, "Ошибка при открытии интерфейса аппарата: " + QString::fromStdString(device->deviceDescription()._device_info), "Ошибка при открытии интерфейса: " + deviceExtension->errorString());
        delete deviceExtension;
        return;
    }

    connect(deviceExtension, &DeviceUiWidget::switchUiToLocal  , this,
            []()
            {
            }
    );
    connect(deviceExtension, &DeviceUiWidget::switchUiToOnboard, this,
            []()
            {
            }
    );
    connect(ui.unwrap().get(), &mccuav::UavUi::typeChanged, this, [name, this]() { onUiTypeChanged(name); });

    _layout->addWidget(deviceExtension);
    _indexMap[*device] = _layout->count() - 1;

    onDeviceChanged(*device);
}

void DeviceUiTool::onUiTypeChanged(mccmsg::Device device)
{
}

void DeviceUiTool::onDeviceRemoved(mccuav::Uav* uav)
{
    if (_indexMap.contains(uav))
    {
        int index = _indexMap[uav];
        _indexMap.remove(uav);

        for (auto key : _indexMap.keys())
        {
            if (_indexMap[key] >= index)
                _indexMap[key]--;
        }

        auto widget = _layout->takeAt(index)->widget();
        _layout->removeWidget(widget);
        delete widget;
    }
}

void DeviceUiTool::onDeviceChanged(mccuav::Uav* device)
{
    if (device != nullptr && _indexMap.contains(device))
    {
        _layout->setCurrentIndex(_indexMap[device]);
        return;
    }
    _layout->setCurrentIndex(0);
}
}
