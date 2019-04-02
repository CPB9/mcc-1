#include "MavlinkToolbarWidget.h"

#include "mcc/uav/Uav.h"
#include "mcc/uav/UavController.h"

#include "mcc/msg/ParamList.h"

#include <QGridLayout>
#include <QToolButton>

namespace mccmav {

MavlinkToolbarWidget::MavlinkToolbarWidget(mccuav::UavController* uavController, mccmsg::Protocol protocol)
    : _protocol(protocol)
    , _uav(nullptr)
{
    auto layout = new QGridLayout();
    layout->setContentsMargins(1, 1, 1, 1);
    setLayout(layout);

    auto btn1 = new QToolButton();
    btn1->setText("Mission");

    auto btn2 = new QToolButton();
    btn2->setText("Land");

    auto btn3 = new QToolButton();
    btn3->setText("Arm/Disarm");

    auto btn4 = new QToolButton();
    btn4->setText("Takeoff");

    connect(uavController, &mccuav::UavController::selectionChanged, this, &MavlinkToolbarWidget::uavSelectionChanged);
    connect(uavController, &mccuav::UavController::uavTmStorageUpdated, this, &MavlinkToolbarWidget::uavTmStorageUpdated);

    connect(btn1, &QToolButton::pressed, this, [this]() { sendCmd("setMode", { QString("Mission") }); });
    connect(btn2, &QToolButton::pressed, this, [this]() { sendCmd("setMode", { QString("Land") }); });
    connect(btn4, &QToolButton::pressed, this, [this]() { sendCmd("setMode", { QString("Takeoff") }); });
    connect(btn3, &QToolButton::pressed, this,
            [this]()
            {
                assert(_uavState.isSome());
                if (_uavState.isNone())
                    return;
                sendCmd("setArmed", { _uavState->armed().unwrap() ? 0 : 1 });
            }
    );


    layout->addWidget(btn1, 0, 0);
    layout->addWidget(btn4, 1, 0);
    layout->addWidget(btn3, 0, 1);
    layout->addWidget(btn2, 1, 1);

    uavSelectionChanged(uavController->selectedUav());
}

void MavlinkToolbarWidget::uavSelectionChanged(mccuav::Uav* uav)
{
    if (_uav == uav)
        return;

    setVisible(uav != nullptr && uav->protocol() == _protocol);
    _uav = uav;
}

void MavlinkToolbarWidget::uavTmStorageUpdated(mccuav::Uav* uav)
{
    if (uav != _uav || uav->tmStorage().isNull())
        return;

    _uavState = uav->tmStorage()->getExtension<mccmsg::TmUavState>();
    if (_uavState.isNone())
        return;

    _uavState->addHandler(std::bind(&MavlinkToolbarWidget::uavStateChanged, this), true);
    uavStateChanged();
}

void MavlinkToolbarWidget::uavStateChanged()
{
    assert(_uavState.isSome());
    updateButtonsState();
}

void MavlinkToolbarWidget::sendCmd(bmcl::StringView command, const mccmsg::CmdParams& params)
{
    assert(_uav);
    if (!_uav)
        return;

    _uavController->sendCmdYY(new mccmsg::CmdParamList(_uav->device(), "Device", command, params));
}

void MavlinkToolbarWidget::updateButtonsState()
{
}

}