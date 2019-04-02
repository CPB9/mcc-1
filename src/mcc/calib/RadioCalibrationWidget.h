#pragma once

#include "mcc/msg/Calibration.h"
#include "mcc/calib/CalibrationControllerAbstract.h"
#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"

class QProgressBar;
class QLabel;
class QVBoxLayout;
class QPushButton;

namespace mcccalib {

class RadioCalibrationWidget : public CalibrationControllerAbstract
{
    Q_OBJECT
public:
    RadioCalibrationWidget(mccuav::UavController* uavController);
    ~RadioCalibrationWidget();

    virtual void start() override;
    virtual void cancel() override;

    virtual void onTraitCalibrationState(const mccmsg::TmCalibrationPtr& msg) override;

private:
    mccui::Rc<mccuav::UavController> _uavController;
    std::vector<QProgressBar*> _channels;
    QLabel* _instructions;
    QLabel* _image;
    QPushButton* _nextButton;
    QPushButton* _skipButton;

    QVBoxLayout* _layout;
    QVBoxLayout* _channelsLayout;
};
}
