#pragma once
#include <QWizard>
#include "mcc/msg/Calibration.h"
#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"

class QComboBox;

namespace mcccalib {

class SensorCalibrationWizard : public QWizard
{
public:
    enum Pages {
        Page_DeviceSelect,
        Page_CalibrateCompass,
        Page_CalibrateGyroscope,
        Page_CalibrateAccelerometer,
        Page_CalibrateLevel,
        Page_CalibrateEsc,
        Page_CalibrateRadio
    };
    SensorCalibrationWizard(mccuav::UavController* uavController);
    ~SensorCalibrationWizard();

    virtual void reject() override;

private:
    mccui::Rc<mccuav::UavController> _uavController;
};
}
