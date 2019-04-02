#pragma once

#include "mcc/msg/Calibration.h"
#include "mcc/calib/CalibrationControllerAbstract.h"
#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"

class QProgressBar;
class QLabel;

namespace mcccalib {

class IndicatorWidget;

class SensorCalibrationWidget : public CalibrationControllerAbstract
{
    Q_OBJECT

public:
    SensorCalibrationWidget(mccuav::UavController* uavController, mccmsg::CalibrationSensor sensor);
    ~SensorCalibrationWidget();

    virtual void onTraitCalibrationState(const mccmsg::TmCalibrationPtr& msg) override;
private slots:
    void onCalibrationState(const mccmsg::TmCalibrationPtr& state);

    virtual void start() override;

    virtual void cancel() override;

private:
    mccui::Rc<mccuav::UavController> _uavController;
    IndicatorWidget* _vehicleDown;
    IndicatorWidget* _vehicleLeft;
    IndicatorWidget* _vehicleNoseDown;
    IndicatorWidget* _vehicleRight;
    IndicatorWidget* _vehicleTailDown;
    IndicatorWidget* _vehicleUpsideDown;
    QProgressBar*    _progressBar;
    QLabel*          _assistMessage;

    int _progress;
};
}
