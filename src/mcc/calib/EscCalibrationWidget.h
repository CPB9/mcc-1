#pragma once

#include "mcc/msg/Calibration.h"
#include "mcc/calib/CalibrationControllerAbstract.h"
#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"

class QLabel;

namespace mcccalib {

class EscCalibrationWidget : public CalibrationControllerAbstract
{
    Q_OBJECT
public:
    EscCalibrationWidget(mccuav::UavController* uavController);
    ~EscCalibrationWidget();

    virtual void start() override;
    virtual void cancel() override;

private slots:
    void onCalibrationState(const mccmsg::TmCalibrationPtr& state);

private:
    mccui::Rc<mccuav::UavController> _uavController;
    QLabel* _instructions;
};
}
