#include "mcc/calib/EscCalibrationWidget.h"
#include "mcc/uav/UavController.h"

#include "mcc/msg/Calibration.h"

#include <QLabel>
#include <QVBoxLayout>

namespace mcccalib {

EscCalibrationWidget::~EscCalibrationWidget()
{
}

EscCalibrationWidget::EscCalibrationWidget(mccuav::UavController* uavController)
    : CalibrationControllerAbstract(mccmsg::CalibrationSensor::Esc)
    , _uavController(uavController)
{
    _instructions = new QLabel();
    auto layout = new QVBoxLayout();
    layout->addWidget(_instructions);
    setLayout(layout);


    connect(_uavController.get(), &mccuav::UavController::traitCalibration, this, &EscCalibrationWidget::onCalibrationState);
}

void EscCalibrationWidget::start()
{
    _uavController->sendCmdYY(new mccmsg::CmdCalibrationStart(device(), sensor()));
}

void EscCalibrationWidget::cancel()
{
    _uavController->sendCmdYY(new mccmsg::CmdCalibrationCancel(device()));
}

void EscCalibrationWidget::onCalibrationState(const mccmsg::TmCalibrationPtr& s)
{
    const mccmsg::Calibration* state = &s->state();
    if (state->_sensor != mccmsg::CalibrationSensor::Esc)
        return;

    _instructions->setText(QString::fromStdString(state->message));

    if (state->done)
    {
        emit completed();
    }
    else if (state->failed && state->waitingForCancel)
    {
        emit cancelled();
    }
    else if (state->failed)
    {
        emit failed();
    }
}
}
