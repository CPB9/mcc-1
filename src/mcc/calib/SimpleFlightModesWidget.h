#pragma once

#include "mcc/calib/CalibrationControllerAbstract.h"
#include "mcc/msg/Calibration.h"
#include "mcc/msg/FwdExt.h"
#include "mcc/msg/ptr/Fwd.h"
#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"

class QLabel;
class QComboBox;

namespace mcccalib {

class SimpleFlightModesWidget : public CalibrationControllerAbstract
{
    Q_OBJECT
public:
    SimpleFlightModesWidget(mccuav::UavController* uavController);
    ~SimpleFlightModesWidget();

    virtual void start() override { }
    virtual void cancel() override { }

    void updateParam(const std::string& id, int value);
public:
    void onCalibrationState(const mccmsg::TmCalibrationPtr& state);
private slots:

    void onModeChannelChanged(int index);
    void onFlightModeChanged(int index);
    void onReturnSwitchChanged();
    void onKillSwitchChanged();
    void onOffborardSwitchChanged();

private:
    void fillChannels(QComboBox* comboBox);
    void fillModes(QComboBox* comboBox);

    void updateModesComboBox(bmcl::Option<int> value, QComboBox* comboBox);
    void updateChannelsComboBox(bmcl::Option<int> value, QComboBox* comboBox);

    void readSettings(int channel);
    void writeVariable(const std::string& trait, const std::string& id, int value);

    void activeFlightModeChanged();

    mccui::Rc<mccuav::UavController> _uavController;

    QLabel* _flightModeLabel1;
    QLabel* _flightModeLabel2;
    QLabel* _flightModeLabel3;
    QLabel* _flightModeLabel4;
    QLabel* _flightModeLabel5;
    QLabel* _flightModeLabel6;

    QComboBox* _modeChannel;
    QComboBox* _flightMode1;
    QComboBox* _flightMode2;
    QComboBox* _flightMode3;
    QComboBox* _flightMode4;
    QComboBox* _flightMode5;
    QComboBox* _flightMode6;

    QComboBox* _returnSwitch;
    QComboBox* _killSwitch;
    QComboBox* _offboardSwitch;

    int _activeFlightMode;
    bmcl::Option<int> _rcFlightMode;  // RC_MAP_FLTMODE
    bmcl::Option<int> _rcRev;         // RC%1_REV
    bmcl::Option<int> _rcMin;         // RC%1_MIN
    bmcl::Option<int> _rcMax;         // RC%1_MAX
    bmcl::Option<int> _rcTrim;        // RC%1_TRIM
    bmcl::Option<int> _rcDz;          // RC%1_DZ

    bool _inited;
    bool _commitUpdates;

    mccmsg::CalibrationFlightModes _flightModes;
};
}
