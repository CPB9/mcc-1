// #include "mcc/ui/px4/SensorCalibrationWizard.h"
// #include "mcc/ui/px4/SensorCalibrationWidget.h"
// #include "mcc/uav/UavController.h"
//
// #include <QLabel>
// #include <QComboBox>
// #include <QCheckBox>
// #include <QHBoxLayout>
// #include <QGroupBox>
// #include <QProgressBar>
//
// namespace mccui {
// namespace px4 {
//

//SensorCalibrationWizard::~SensorCalibrationWizard()
//{
//}
//
//
// class SensorStatusCheckBox : public QWidget
// {
// public:
//     SensorStatusCheckBox(const mcc::misc::Calibration::Sensor sensor, const VarConditions& vars, const QString& desc)
//         : _sensor(sensor)
//         , _vars(vars)
//     {
//         auto layout = new QHBoxLayout();
//         setLayout(layout);
//
//         _status = new QLabel();
//         _status->setFixedSize(20, 20);
//
//         _checkBox = new QCheckBox(desc);
//         layout->addWidget(_status);
//         layout->addWidget(_checkBox);
//         layout->addStretch();
//
//         setCalibrated(true);
//
//         connect(mccContext, &mcc::ui::core::Context::systemStarted, this,
//             [this]()
//             {
//                 connect(mccContext->exchangeService(), &mcc::ui::mccuav::ExchangeService::tmParamList, this, &SensorStatusCheckBox::onTmParamList);
//             }
//         );
//     }
//
// public slots:
//     void setDevice(const mccmsg::Device& device)
//     {
//         _device = device;
//
//         mccmsg::CmdParams params = { "px4", 0, 65 };
//         _uavController->sendCmd<mccmsg::cmd::ParamList>(mccmsg::Device(device), "Tm", "read", params);
//     }
//
// private slots:
//     void onTmParamList(const mccmsg::tm::ParamList& params)
//     {
//         if (_device != params.device())
//             return;
//
//         for (auto p : params.params())
//         {
//             if (p.status() == _vars.front().id)
//             {
//                 setCalibrated(_vars.front().func(p.value()));
//             }
//         }
//     }
//
// private:
//     void setCalibrated(bool calibrated)
//     {
//         QString color = calibrated ? "green" : "red";
//
//         QString css = QString("background-color: %1;").arg(color);
//         _status->setStyleSheet(css);
//     }
//
// private:
//     mcc::misc::Calibration::Sensor _sensor;
//     QLabel*                        _status;
//     QCheckBox*                     _checkBox;
//     mccmsg::Device                 _device;
//     const VarConditions            _vars;
// };
//
// class DeviceSelectorPage : public QWizardPage
// {
// public:
//     DeviceSelectorPage()
//     {
//         using mcc::misc::Calibration;
//
//         setTitle("Выбор устройства");
//         QVBoxLayout* layout = new QVBoxLayout();
//         layout->addWidget(new QLabel("Выберите устройство: "));
//         _devicesList = new QComboBox();
//         layout->addWidget(_devicesList);
//         setLayout(layout);
//
//         auto gb = new QGroupBox("Виды калибровки");
//         auto kindLayout = new QVBoxLayout();
//         gb->setLayout(kindLayout);
//
//         VarCondition::Condition magCondition = [](const mcc::misc::NetVariant& value) { return value.toInt() != 0; };
//
//         _compassCheckBox = new SensorStatusCheckBox(Calibration::Sensor::Magnetometer, { VarCondition("CAL_MAG0_ID", magCondition) }, "Компас");
//
//         kindLayout->addWidget(_compassCheckBox);
//     //        kindLayout->addWidget(new SensorStatusCheckBox(Calibration::Sensor::Gyroscope,      "CAL_GYRO0_ID", "Гироскоп"));
//     //        kindLayout->addWidget(new SensorStatusCheckBox(Calibration::Sensor::Accelerometer,  "CAL_ACC0_ID", "Акселерометр"));
//     //        kindLayout->addWidget(new SensorStatusCheckBox(Calibration::Sensor::Level,          "SENS_BOARD_X_OFF, SENS_BOARD_Y_OFF, SENS_BOARD_Z_OFF", "Уровень горизонта"));
//     //        kindLayout->addWidget(new SensorStatusCheckBox(Calibration::Sensor::Esc,            "", "ESC"));
//     //        kindLayout->addWidget(new SensorStatusCheckBox(Calibration::Sensor::Radio,          "", "Радио"));
//
//         layout->addWidget(gb);
//
//         connect(_devicesList, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
//             [this]()
//             {
//                 mccmsg::Device device = mccmsg::Device(_devicesList->currentData().toString());
// //                _compassCheckBox->setDevice(device);
// //                _gyroCheckBox->setDevice(device);
// //                _accelCheckBox->setDevice(device);
// //                _levelCheckBox->setDevice(device);
// //                _escCheckBox->setDevice(device);
// //                _radioCheckBox->setDevice(device);
//             }
//         );
//
//         registerField("currentDevice", _devicesList, "currentData", "currentIndexChanged");
//     }
//
//     virtual void initializePage() override
//     {
//         _devicesList->clear();
//         for (auto it : _uavController->devicesList())
//         {
//             _devicesList->addItem(QString::fromStdString(it->deviceDescription()._device_info), it->name());
//         }
//     }
//
// private:
//     QComboBox* _devicesList;
//     SensorStatusCheckBox* _compassCheckBox;
//     SensorStatusCheckBox* _gyroCheckBox;
//     SensorStatusCheckBox* _accelCheckBox;
//     SensorStatusCheckBox* _levelCheckBox;
//     SensorStatusCheckBox* _escCheckBox;
//     SensorStatusCheckBox* _radioCheckBox;
// };
//
// class SensorCalibrationPage : public QWizardPage
// {
// public:
//     SensorCalibrationPage(const QString& name, mcc::misc::Calibration::Sensor sensor)
//     {
//         setTitle(QString("Калибровка: %1").arg(name));
//
//         _widget = new SensorCalibrationWidget(sensor);
//         QVBoxLayout* layout = new QVBoxLayout();
//         layout->addWidget(_widget);
//         setLayout(layout);
//     }
//
//     virtual void initializePage() override
//     {
//         auto currentDevice = field("currentDevice");
//
//         _uavController->sendCmd<mccmsg::cmd::CalibrationStart>(mccmsg::Device(currentDevice.toString()), _widget->sensor());
//     }
// private:
//     SensorCalibrationWidget* _widget;
// };
//
// class EscCalibrationPage : public QWizardPage
// {
// public:
//     EscCalibrationPage()
//     {
//         setTitle("Калибровка ESC");
//
//         QVBoxLayout* layout = new QVBoxLayout();
// //        layout->addWidget(warningLabel);
//         setLayout(layout);
//     }
//
//
// };
//
// class RadioCalibrationPage : public QWizardPage
// {
// public:
//     RadioCalibrationPage()
//     {
//         _layout = new QVBoxLayout();
//         setLayout(_layout);
//         _instructions = new QLabel();
//         _image = new QLabel();
//
//         _layout->addWidget(_instructions);
//         _layout->addWidget(_image);
//
//         connect(mccContext, &mcc::ui::core::Context::systemStarted, this,
//                 [this]()
//             {
//                 connect(mccContext->exchangeService(), &mcc::ui::mccuav::ExchangeService::traitCalibration, this, &RadioCalibrationPage::onCalibrationState);
//             }
//         );
//     }
//
//     virtual void initializePage() override
//     {
//         auto currentDevice = field("currentDevice");
//
//         _uavController->sendCmd<mccmsg::cmd::CalibrationStart>(mccmsg::Device(currentDevice.toString()), mcc::misc::Calibration::Sensor::Radio);
//     }
// private slots:
//     void onCalibrationState(const mcc::misc::Calibration& state)
//     {
//         if (state._sensor != mcc::misc::Calibration::Sensor::Radio)
//             return;
//
//         _instructions->setText(QString::fromStdString(state.message));
//         if (!state.image.empty())
//         {
//             QString res = QString(":/resources/%1").arg(state.image.c_str());
//             _image->setPixmap(res);
//         }
//
//         if (_channels.empty())
//         {
//             for (int i = 0; i < state.pwmValues.size(); ++i)
//             {
//                 auto bar = new QProgressBar();
//                 bar->setMinimum(800);
//                 bar->setMaximum(2200);
//                 _channels.push_back(bar);
//                 //_layout->addWidget(bar);
//             }
//         }
//
//         for (int i = 0; i < state.pwmValues.size(); ++i)
//         {
//             auto bar = _channels[i];
//             if (state.pwmValues[i].isNone())
//             {
//                 bar->setEnabled(false);
//                 continue;
//             }
//             bar->setEnabled(true);
//             bar->setValue(*state.pwmValues[i]);
//         }
//     }
//
// private:
//     std::vector<QProgressBar*> _channels;
//     QLabel* _instructions;
//     QLabel* _image;
//
//     QVBoxLayout* _layout;
// };
//
// SensorCalibrationWizard::SensorCalibrationWizard(mccuav::UavController* uavController)
//    , _uavController(uavController)
// {
//     using mcc::misc::Calibration;
//
//     setObjectName("SensorCalibrationWizard");
//     setWindowTitle("Калибровка сенсоров");
//
//     setPage(Page_DeviceSelect, new DeviceSelectorPage());
//     //setPage(Page_CalibrateCompass, new SensorCalibrationPage("Компас",       Calibration::Sensor::Magnetometer));
//     //setPage(Page_CalibrateGyroscope, new SensorCalibrationPage("Гироскоп",     Calibration::Sensor::Gyroscope));
//     //setPage(Page_CalibrateAccelerometer, new SensorCalibrationPage("Акселерометр", Calibration::Sensor::Accelerometer));
//     //setPage(Page_CalibrateLevel, new SensorCalibrationPage("Уровень горизонта", Calibration::Sensor::Level));
//     //setPage(Page_CalibrateEsc, new EscCalibrationPage());
//     setPage(Page_CalibrateRadio, new RadioCalibrationPage());
// }
//
// void SensorCalibrationWizard::reject()
// {
//     auto currentDevice = field("currentDevice");
//     if(currentDevice.isValid())
//         _uavController->sendCmd<mccmsg::cmd::CalibrationCancel>(mccmsg::Device(currentDevice.toString()));
//
//     QWizard::reject();
// }
//
// }
// }
//
