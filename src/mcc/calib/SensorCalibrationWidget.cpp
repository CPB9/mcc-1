#include "mcc/calib/SensorCalibrationWidget.h"
#include "mcc/calib/IndicatorWidget.h"

#include <QGridLayout>
#include <QPixmap>
#include <QProgressBar>
#include <QLabel>
#include "mcc/ui/FlowLayout.h"

namespace mcccalib {

SensorCalibrationWidget::~SensorCalibrationWidget()
{
}

SensorCalibrationWidget::SensorCalibrationWidget(mccuav::UavController* uavController, mccmsg::CalibrationSensor sensor)
    : CalibrationControllerAbstract(sensor)
    , _uavController(uavController)
{
    setObjectName("SensorCalibration");
    setWindowTitle("Калибровка датчиков");

    _progressBar = new QProgressBar(this);
    _progressBar->setValue(0);
    _progress = 0;

    _assistMessage = new QLabel(this);

    _vehicleDown = new IndicatorWidget(this, ":/resources/VehicleDown.png", ":/resources/VehicleDownRotate.png");
    _vehicleLeft = new IndicatorWidget(this, ":/resources/VehicleLeft.png", ":/resources/VehicleLeftRotate.png");
    _vehicleNoseDown = new IndicatorWidget(this, ":/resources/VehicleNoseDown.png", ":/resources/VehicleNoseDownRotate.png");
    _vehicleRight = new IndicatorWidget(this, ":/resources/VehicleRight.png", ":/resources/VehicleRightRotate.png");
    _vehicleTailDown = new IndicatorWidget(this, ":/resources/VehicleTailDown.png", ":/resources/VehicleTailDownRotate.png");
    _vehicleUpsideDown = new IndicatorWidget(this, ":/resources/VehicleUpsideDown.png", ":/resources/VehicleUpsideDownRotate.png");

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(_progressBar);
    layout->addWidget(_assistMessage);

    auto flowLayout = new FlowLayout();
    flowLayout->addWidget(_vehicleDown);
    flowLayout->addWidget(_vehicleLeft);
    flowLayout->addWidget(_vehicleUpsideDown);
    flowLayout->addWidget(_vehicleNoseDown);
    flowLayout->addWidget(_vehicleTailDown);
    flowLayout->addWidget(_vehicleRight);

    layout->addLayout(flowLayout);
    layout->addStretch();

    setLayout(layout);

//     connect(mccContext, &mccui::Context::systemStarted, this,
//             [this]()
//     {
//         connect(mccContext->exchangeService(), &mccuav::ExchangeService::traitCalibration, this, &SensorCalibrationWidget::onCalibrationState);
//     }
//     );

    //setStyleSheet("background-color: gray");
}


void SensorCalibrationWidget::onTraitCalibrationState(const mccmsg::TmCalibrationPtr& msg)
{
    onCalibrationState(msg);
}

void SensorCalibrationWidget::onCalibrationState(const mccmsg::TmCalibrationPtr& s)
{
    if (s->device() != device())
    {
        return;
    }

    const mccmsg::Calibration* state = &s->state();
    if (state->_sensor != sensor())
    {
        return;
    }

    _assistMessage->setText(QString::fromStdString(state->message));
    _progressBar->setValue(state->progress);

    _vehicleDown->setStatus(state->calDownSide);
    _vehicleLeft->setStatus(state->calLeftSide);
    _vehicleNoseDown->setStatus(state->calNoseDownSide);
    _vehicleRight->setStatus(state->calRightSide);
    _vehicleTailDown->setStatus(state->calTailDownSide);
    _vehicleUpsideDown->setStatus(state->calUpsideDownSide);

    if (state->failed)
    {
        if (state->waitingForCancel)
        {
            _assistMessage->setText("Cancelled by user");
            emit cancelled();
        }
        else
        {
            _assistMessage->setText("Failed");
            emit failed();
        }
    }
    if (state->done)
    {
        Q_ASSERT(state->progress == 100);
        emit completed();
    }
}

void SensorCalibrationWidget::start()
{
    _uavController->sendCmdYY(new mccmsg::CmdCalibrationStart(device(), sensor()));
}

void SensorCalibrationWidget::cancel()
{
    _uavController->sendCmdYY(new mccmsg::CmdCalibrationCancel(device()));
}
}
