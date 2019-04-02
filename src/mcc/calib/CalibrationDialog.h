#pragma once
#include "mcc/Config.h"
#include "mcc/msg/FwdExt.h"
#include "mcc/msg/Calibration.h"
#include "mcc/ui/Dialog.h"
#include "mcc/uav/Rc.h"
#include "mcc/uav/Fwd.h"
#include <bmcl/Option.h>

class QWidget;
class QListWidget;
class QStackedWidget;
class QPushButton;

namespace mcccalib {

class CalibrationDialogPage;

class MCC_CALIB_DECLSPEC CalibrationDialog : public QWidget
{
    Q_OBJECT
public:
    CalibrationDialog(mccuav::UavController* uavController, QWidget* parent, bmcl::Option<mccmsg::Device> dev = bmcl::None);
    ~CalibrationDialog();

    void addPage(const QString& section, const QPixmap& pixmap, CalibrationDialogPage* contents);
    void addPage(const QString& section, QWidget* contents);
    void onTraitCalibration(const mccmsg::TmCalibrationPtr& msg);
    void onCommonCalibrationStatus(const mccmsg::TmCommonCalibrationStatusPtr& status);

private slots:
    void selectionChanged(mccuav::Uav* uav);
    void deviceFirmwareLoaded(mccuav::Uav* uav);
    void setStatus(mccmsg::CalibrationSensor sensor, bool status);
    void calibrationStarted(mccmsg::CalibrationSensor sensor);
    void calibrationCancelled(mccmsg::CalibrationSensor sensor);

    void reject();
private:
    mccuav::Rc<mccuav::UavController> _uavController;
    bmcl::Option<mccmsg::Device> _device;
    mccuav::Uav* _uav;
    QListWidget* _sectionsWidget;
    QStackedWidget* _contentsWidget;
    QPushButton* _closeButton;
};
}
