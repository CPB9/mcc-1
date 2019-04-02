#include "mcc/calib/SimpleFlightModesWidget.h"

#include <fmt/format.h>

#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QFormLayout>

#include "mcc/msg/ptr/Tm.h"
#include "mcc/msg/ParamList.h"
#include "mcc/msg/Tm.h"
#include "mcc/msg/FwdExt.h"

namespace mcccalib {

SimpleFlightModesWidget::~SimpleFlightModesWidget()
{
}

SimpleFlightModesWidget::SimpleFlightModesWidget(mccuav::UavController* uavController)
    : CalibrationControllerAbstract(mccmsg::CalibrationSensor::Radio)
    , _uavController(uavController)
    , _activeFlightMode(-1)
    , _inited(false)
    , _commitUpdates(true)
{
    auto mainLayout = new QGridLayout();
    setLayout(mainLayout);
    mainLayout->addWidget(new QLabel("Flight modes setup is used to configure the transmitter switches associated with Flight Modes"), 0, 0, 1, 2);

    _modeChannel = new QComboBox();
    _flightMode1 = new QComboBox();
    _flightMode2 = new QComboBox();
    _flightMode3 = new QComboBox();
    _flightMode4 = new QComboBox();
    _flightMode5 = new QComboBox();
    _flightMode6 = new QComboBox();

    _flightModeLabel1 = new QLabel("Flight mode 1:");
    _flightModeLabel2 = new QLabel("Flight mode 2:");
    _flightModeLabel3 = new QLabel("Flight mode 3:");
    _flightModeLabel4 = new QLabel("Flight mode 4:");
    _flightModeLabel5 = new QLabel("Flight mode 5:");
    _flightModeLabel6 = new QLabel("Flight mode 6:");

    auto flightModesLayout = new QFormLayout();
    flightModesLayout->addRow("Mode channel:", _modeChannel);
    flightModesLayout->addRow(_flightModeLabel1, _flightMode1);
    flightModesLayout->addRow(_flightModeLabel2, _flightMode2);
    flightModesLayout->addRow(_flightModeLabel3, _flightMode3);
    flightModesLayout->addRow(_flightModeLabel4, _flightMode4);
    flightModesLayout->addRow(_flightModeLabel5, _flightMode5);
    flightModesLayout->addRow(_flightModeLabel6, _flightMode6);

    auto flightModesGroupBox = new QGroupBox();
    flightModesGroupBox->setTitle("Flight Modes");
    flightModesGroupBox->setLayout(flightModesLayout);

    _returnSwitch = new QComboBox();
    _killSwitch = new QComboBox();
    _offboardSwitch = new QComboBox();

    auto switchSettingsLayout = new QFormLayout();
    switchSettingsLayout->addRow("Return switch:", _returnSwitch);
    switchSettingsLayout->addRow("Kill switch:", _killSwitch);
    switchSettingsLayout->addRow("Offboard switch:", _offboardSwitch);

    auto switchSettingsGroupBox = new QGroupBox();
    switchSettingsGroupBox->setTitle("Switch Settings");
    switchSettingsGroupBox->setLayout(switchSettingsLayout);

    mainLayout->addWidget(flightModesGroupBox, 1 ,0);
    mainLayout->addWidget(switchSettingsGroupBox, 1, 1);
    mainLayout->setRowStretch(2, 1);

    fillChannels(_modeChannel);
    fillChannels(_returnSwitch);
    fillChannels(_killSwitch);
    fillChannels(_offboardSwitch);
    fillModes(_flightMode1);
    fillModes(_flightMode2);
    fillModes(_flightMode3);
    fillModes(_flightMode4);
    fillModes(_flightMode5);
    fillModes(_flightMode6);


    //connect(mccContext->exchangeService(), &mccuav::ExchangeService::traitCalibration, this, &SimpleFlightModesWidget::onCalibrationState);

    connect(_modeChannel, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SimpleFlightModesWidget::onModeChannelChanged);
    connect(_flightMode1, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SimpleFlightModesWidget::onFlightModeChanged);
    connect(_flightMode2, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SimpleFlightModesWidget::onFlightModeChanged);
    connect(_flightMode3, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SimpleFlightModesWidget::onFlightModeChanged);
    connect(_flightMode4, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SimpleFlightModesWidget::onFlightModeChanged);
    connect(_flightMode5, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SimpleFlightModesWidget::onFlightModeChanged);
    connect(_flightMode6, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SimpleFlightModesWidget::onFlightModeChanged);

    connect(_returnSwitch,   static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SimpleFlightModesWidget::onReturnSwitchChanged);
    connect(_killSwitch,     static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SimpleFlightModesWidget::onKillSwitchChanged);
    connect(_offboardSwitch, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &SimpleFlightModesWidget::onOffborardSwitchChanged);
}


void SimpleFlightModesWidget::updateModesComboBox(bmcl::Option<int> value, QComboBox* comboBox)
{
    int tmp = -1;
    if (value.isSome())
        tmp = value.unwrap();

    //int index = comboBox->currentIndex();
    int current = comboBox->currentData().toInt();

    if (current != tmp)
        comboBox->setCurrentIndex(comboBox->findData(tmp));
}

void SimpleFlightModesWidget::updateChannelsComboBox(bmcl::Option<int> value, QComboBox* comboBox)
{
    int tmp = 0;
    if (value.isSome())
        tmp = value.unwrap();

    //int index = comboBox->currentIndex();
    int current = comboBox->currentData().toInt();

    if (current != tmp)
        comboBox->setCurrentIndex(comboBox->findData(tmp));
}

void SimpleFlightModesWidget::onCalibrationState(const mccmsg::TmCalibrationPtr& s)
{
    if (device() != s->device())
        return;

    const mccmsg::Calibration* state = &s->state();
    if (state->_sensor != mccmsg::CalibrationSensor::FlightModes)
        return;

    _flightModes = state->flightModes;

    _rcFlightMode = state->flightModes.flightModeChannel;
    _commitUpdates = false;

    updateChannelsComboBox(_rcFlightMode, _modeChannel);
    updateModesComboBox(state->flightModes.mode1, _flightMode1);
    updateModesComboBox(state->flightModes.mode2, _flightMode2);
    updateModesComboBox(state->flightModes.mode3, _flightMode3);
    updateModesComboBox(state->flightModes.mode4, _flightMode4);
    updateModesComboBox(state->flightModes.mode5, _flightMode5);
    updateModesComboBox(state->flightModes.mode6, _flightMode6);
    updateChannelsComboBox(state->flightModes.killSwitchChannel, _killSwitch);
    updateChannelsComboBox(state->flightModes.offboardSwitchChannel, _offboardSwitch);
    updateChannelsComboBox(state->flightModes.returnSwitchChannel, _returnSwitch);
    _commitUpdates = true;

    if(_rcFlightMode.isNone())
    {
        _activeFlightMode = 0;
        activeFlightModeChanged();
        return;
    }

    int flightModeChannel = *_rcFlightMode - 1;

    if (_rcRev.isNone() || _rcMin.isNone() || _rcMax.isNone() || _rcTrim.isNone() || _rcDz.isNone())
        return;

    int pwmRev = *_rcRev;
    int pwmMin = *_rcMin;
    int pwmMax = *_rcMax;
    int pwmTrim = *_rcTrim;
    int pwmDz = *_rcDz;

    if (flightModeChannel < 0) {
        return;
    }

    _activeFlightMode = 0;
    auto channelValue = state->pwmValues[flightModeChannel];

    if (channelValue.isSome())
    {
        /* the half width of the range of a slot is the total range
        * divided by the number of slots, again divided by two
        */

        const unsigned num_slots = 6;

        const float slot_width_half = 2.0f / num_slots / 2.0f;

        /* min is -1, max is +1, range is 2. We offset below min and max */
        const float slot_min = -1.0f - 0.05f;
        const float slot_max = 1.0f + 0.05f;

        /* the slot gets mapped by first normalizing into a 0..1 interval using min
        * and max. Then the right slot is obtained by multiplying with the number of
        * slots. And finally we add half a slot width to ensure that integer rounding
        * will take us to the correct final index.
        */

        float calibrated_value;

        if (*channelValue > (pwmTrim + pwmDz))
        {
            calibrated_value = (*channelValue - pwmTrim - pwmDz) / (float)(pwmMax - pwmTrim - pwmDz);

        }
        else if (*channelValue < (pwmTrim - pwmDz))
        {
            calibrated_value = (*channelValue - pwmTrim + pwmDz) / (float)(pwmTrim - pwmMin - pwmDz);
        }
        else
        {
            /* in the configured dead zone, output zero */
            calibrated_value = 0.0f;
        }

        calibrated_value *= pwmRev;

        _activeFlightMode = (((((calibrated_value - slot_min) * num_slots) + slot_width_half) /
            (slot_max - slot_min)) + (1.0f / num_slots));

        if (_activeFlightMode >= static_cast<int>(num_slots)) {
            _activeFlightMode = num_slots - 1;
        }

        // move to 1-based index
        _activeFlightMode++;

        activeFlightModeChanged();
    }
}

void SimpleFlightModesWidget::updateParam(const std::string& id, int value)
{
    if (id == "RC_MAP_FLTMODE")
    {
        int newFltMode = value;

        if(_rcFlightMode.isNone() && newFltMode > 0)
        {
            _rcFlightMode = newFltMode;
            readSettings(*_rcFlightMode);

            _modeChannel->setCurrentIndex(_modeChannel->findData(newFltMode));
        }
        else if (_rcFlightMode.isSome() && newFltMode == 0)
        {
            _rcFlightMode = bmcl::None;
            _rcRev = bmcl::None;
            _rcMin = bmcl::None;
            _rcMax = bmcl::None;
            _rcTrim = bmcl::None;
            _rcDz = bmcl::None;

            _modeChannel->setCurrentIndex(_modeChannel->findData(newFltMode));
        }
        else if (_rcFlightMode.isSome() && *_rcFlightMode != newFltMode)
        {
            _rcFlightMode = newFltMode;
            readSettings(newFltMode);

            _modeChannel->setCurrentIndex(_modeChannel->findData(newFltMode));
        }
    }

    if (_rcFlightMode.isNone())
        return;

    QString rcRev = QString("RC%1_REV").arg(*_rcFlightMode);
    QString rcMin = QString("RC%1_MIN").arg(*_rcFlightMode);
    QString rcMax = QString("RC%1_MAX").arg(*_rcFlightMode);
    QString rcTrim = QString("RC%1_TRIM").arg(*_rcFlightMode);
    QString rcDz = QString("RC%1_DZ").arg(*_rcFlightMode);

    if (id == rcRev.toStdString())
    {
        _rcRev = value;
    }
    else if (id == rcMin.toStdString())
    {
        _rcMin = value;
    }
    else if (id == rcMax.toStdString())
    {
        _rcMax = value;
    }
    else if (id == rcTrim.toStdString())
    {
        _rcTrim = value;
    }
    else if (id == rcDz.toStdString())
    {
        _rcDz = value;
    }
}

void SimpleFlightModesWidget::onModeChannelChanged(int index)
{
    if (!_commitUpdates)
        return;
    mccmsg::CalibrationFlightModes flightModes;
    flightModes.flightModeChannel = _modeChannel->currentData().toInt();
    _uavController->sendCmdYY(new mccmsg::CmdCalibrationStart(device(), sensor(), mccmsg::CalibrationCmd::SetFlightModes, flightModes));
}

void SimpleFlightModesWidget::onFlightModeChanged(int index)
{
    if (!_commitUpdates)
        return;

    mccmsg::CalibrationFlightModes flightModes;
    QComboBox* src = static_cast<QComboBox*>(sender());
    bmcl::Option<int> value = src->currentData().toInt();

    if (src == _flightMode1)
        flightModes.mode1 = value;
    else if (src == _flightMode2)
        flightModes.mode2 = value;
    else if (src == _flightMode3)
        flightModes.mode3 = value;
    else if (src == _flightMode4)
        flightModes.mode4 = value;
    else if (src == _flightMode5)
        flightModes.mode5 = value;
    else if (src == _flightMode6)
        flightModes.mode6 = value;

    _uavController->sendCmdYY(new mccmsg::CmdCalibrationStart(device(), sensor(), mccmsg::CalibrationCmd::SetFlightModes, flightModes));
}

void SimpleFlightModesWidget::onReturnSwitchChanged()
{
    if (!_commitUpdates)
        return;

    mccmsg::CalibrationFlightModes flightModes;
    flightModes.returnSwitchChannel = _returnSwitch->currentData().toInt();
    _uavController->sendCmdYY(new mccmsg::CmdCalibrationStart(device(), sensor(), mccmsg::CalibrationCmd::SetFlightModes, flightModes));
}

void SimpleFlightModesWidget::onKillSwitchChanged()
{
    if (!_commitUpdates)
        return;

    mccmsg::CalibrationFlightModes flightModes;
    flightModes.killSwitchChannel = _killSwitch->currentData().toInt();
    _uavController->sendCmdYY(new mccmsg::CmdCalibrationStart(device(), sensor(), mccmsg::CalibrationCmd::SetFlightModes, flightModes));

}

void SimpleFlightModesWidget::onOffborardSwitchChanged()
{
    if (!_commitUpdates)
        return;

    mccmsg::CalibrationFlightModes flightModes;
    flightModes.offboardSwitchChannel = _offboardSwitch->currentData().toInt();
    _uavController->sendCmdYY(new mccmsg::CmdCalibrationStart(device(), sensor(), mccmsg::CalibrationCmd::SetFlightModes, flightModes));
}

void SimpleFlightModesWidget::fillChannels(QComboBox* comboBox)
{
    comboBox->clear();
    comboBox->addItem("Unassigned", 0);
    const int channelsCount = 18;
    for (int i = 0; i < channelsCount; ++i)
    {
        int channel = i + 1;
        comboBox->addItem(QString::number(channel), channel);
    }
}

void SimpleFlightModesWidget::fillModes(QComboBox* comboBox)
{
    comboBox->clear();
    comboBox->addItem("Unassigned", -1);
    comboBox->addItem("Manual", 0);
    comboBox->addItem("Altitude", 1);
    comboBox->addItem("Position", 2);
    comboBox->addItem("Mission", 3);
    comboBox->addItem("Hold", 4);
    comboBox->addItem("Return", 5);
    comboBox->addItem("Acro", 6);
    comboBox->addItem("Offboard", 7);
    comboBox->addItem("Stabilized", 8);
    comboBox->addItem("Rattitude", 9);
    comboBox->addItem("Takeoff", 10);
    comboBox->addItem("Land", 11);
    comboBox->addItem("Follow Me", 12);
}

void SimpleFlightModesWidget::readSettings(int channel)
{
    Q_ASSERT(_rcFlightMode.isSome());

    int mode = *_rcFlightMode;
    std::string rcRev  = fmt::format("RC{}_REV",  mode);
    std::string rcMin  = fmt::format("RC{}_MIN",  mode);
    std::string rcMax  = fmt::format("RC{}_MAX",  mode);
    std::string rcTrim = fmt::format("RC{}_TRIM", mode);
    std::string rcDz   = fmt::format("RC{}_DZ",   mode);

    std::vector<std::string> vars = { rcRev, rcMin, rcMax, rcTrim, rcDz };
    _uavController->sendCmdYY(new mccmsg::CmdParamRead(device(), "Radio Calibration", vars));

    std::vector<std::string> vars2 = { "COM_FLTMODE1", "COM_FLTMODE2", "COM_FLTMODE3", "COM_FLTMODE4", "COM_FLTMODE5", "COM_FLTMODE6" };
    _uavController->sendCmdYY(new mccmsg::CmdParamRead(device(), "Miscellaneous", std::move(vars2)));

}

void SimpleFlightModesWidget::writeVariable(const std::string& trait, const std::string& id, int value)
{
    std::vector<std::pair<std::string, mccmsg::NetVariant>> args;
    args.push_back(std::make_pair(id, value));
    _uavController->sendCmdYY(new mccmsg::CmdParamWrite(device(), "", std::move(args)));
}

void SimpleFlightModesWidget::activeFlightModeChanged()
{
    const QString activeStylesheet = "color: brown;";

    _flightModeLabel1->setStyleSheet("");
    _flightModeLabel2->setStyleSheet("");
    _flightModeLabel3->setStyleSheet("");
    _flightModeLabel4->setStyleSheet("");
    _flightModeLabel5->setStyleSheet("");
    _flightModeLabel6->setStyleSheet("");

    switch (_activeFlightMode)
    {
    case 1: _flightModeLabel1->setStyleSheet(activeStylesheet); break;
    case 2: _flightModeLabel2->setStyleSheet(activeStylesheet); break;
    case 3: _flightModeLabel3->setStyleSheet(activeStylesheet); break;
    case 4: _flightModeLabel4->setStyleSheet(activeStylesheet); break;
    case 5: _flightModeLabel5->setStyleSheet(activeStylesheet); break;
    case 6: _flightModeLabel6->setStyleSheet(activeStylesheet); break;
    }
}
}
