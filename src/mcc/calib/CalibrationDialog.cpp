#include "mcc/calib/CalibrationDialog.h"
// #include "mcc/net/protocols/mavlink/Firmware.h"

#include "mcc/uav/UavController.h"
#include "mcc/uav/ChannelsController.h"

#include <QApplication>
#include <QListWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include "mcc/calib/CalibrationDialogPage.h"
#include "mcc/calib/SensorCalibrationWidget.h"
#include "mcc/calib/RadioCalibrationWidget.h"
#include "mcc/calib/EscCalibrationWidget.h"
#include "mcc/calib/SimpleFlightModesWidget.h"

MCC_INIT_QRESOURCES(px4);

namespace mcccalib {

CalibrationDialog::~CalibrationDialog()
{
}

CalibrationDialog::CalibrationDialog(mccuav::UavController* uavController, QWidget* parent, bmcl::Option<mccmsg::Device> dev)
    : QWidget(parent)
    , _uavController(uavController)
{
    _device = dev;
    _sectionsWidget = new QListWidget(this);
    _sectionsWidget->setViewMode(QListView::ListMode);
    _sectionsWidget->setIconSize(QSize(96, 84));
    _sectionsWidget->setMovement(QListView::Static);
    _sectionsWidget->setMaximumWidth(128);
    _sectionsWidget->setSpacing(12);

    _contentsWidget = new QStackedWidget();

    auto horizontalLayout = new QHBoxLayout();
    horizontalLayout->addWidget(_sectionsWidget);
    horizontalLayout->addWidget(_contentsWidget , 1);

    connect(_sectionsWidget, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem* current, QListWidgetItem* previous)
            {
                if (current == previous)
                    return;
                _contentsWidget->setCurrentIndex(_sectionsWidget->row(current));
            }
    );

    setLayout(horizontalLayout);

    addPage("Компас", QPixmap(), new CalibrationDialogPage(new SensorCalibrationWidget(uavController, mccmsg::CalibrationSensor::Magnetometer)));
    addPage("Гироскоп", QPixmap(), new CalibrationDialogPage(new SensorCalibrationWidget(uavController, mccmsg::CalibrationSensor::Gyroscope)));
    addPage("Акселерометр", QPixmap(), new CalibrationDialogPage(new SensorCalibrationWidget(uavController, mccmsg::CalibrationSensor::Accelerometer)));
        //SENS_BOARD_Y_OFF, SENS_BOARD_Z_OFF
    addPage("Горизонт", QPixmap(), new CalibrationDialogPage(new SensorCalibrationWidget(uavController, mccmsg::CalibrationSensor::Level)));

    addPage("ESC", QPixmap(), new CalibrationDialogPage(new EscCalibrationWidget(uavController)));
    //    VarCondition::Condition magCondition = [](const mccmsg::NetVariant& value) { return value.toInt() != 0; };
    addPage("Пульт", QPixmap(), new CalibrationDialogPage(new RadioCalibrationWidget(uavController)));
    addPage("Режимы полета", new SimpleFlightModesWidget(uavController));
        //addPage("Safety", new SafetyWidget());

    using mccuav::UavController;

    auto manager = _uavController.get();
    connect(manager, &UavController::selectionChanged, this, &CalibrationDialog::selectionChanged);
    connect(manager, &UavController::uavFirmwareLoaded, this, &CalibrationDialog::deviceFirmwareLoaded);
}

void CalibrationDialog::addPage(const QString& section, const QPixmap& pixmap, CalibrationDialogPage* contents)
{
    if(_device.isSome())
        contents->setDevice(_device.unwrap());

    QListWidgetItem *configButton = new QListWidgetItem(_sectionsWidget);
    configButton->setIcon(QApplication::style()->standardPixmap(QStyle::SP_MessageBoxCritical));
    configButton->setText(section);
    configButton->setTextAlignment(Qt::AlignLeft);
    configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    _contentsWidget->addWidget(contents);

    connect(contents, &CalibrationDialogPage::started, this, [this](bool started) { _sectionsWidget->setEnabled(!started); });
    connect(contents->controller(), &CalibrationControllerAbstract::calibratedChanged, this,
            [this, contents, configButton]()
            {
                QIcon icon = contents->controller()->calibrated() ? QApplication::style()->standardPixmap(QStyle::SP_MessageBoxInformation) : QApplication::style()->standardPixmap(QStyle::SP_MessageBoxCritical);
                configButton->setIcon(icon);
            }
    );

    connect(contents->controller(), &CalibrationControllerAbstract::completed, this,
            [this, contents, configButton]()
            {
                _sectionsWidget->setEnabled(true);
            }
    );

    connect(contents->controller(), &CalibrationControllerAbstract::failed, this,
            [this, contents, configButton]()
            {
                _sectionsWidget->setEnabled(true);
            }
    );

    connect(contents->controller(), &CalibrationControllerAbstract::cancelled, this,
            [this, contents, configButton]()
            {
                _sectionsWidget->setEnabled(true);
            }
    );
}

void CalibrationDialog::addPage(const QString& section, QWidget* contents)
{
    QListWidgetItem *configButton = new QListWidgetItem(_sectionsWidget);
    configButton->setText(section);
    configButton->setTextAlignment(Qt::AlignLeft);
    configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    _contentsWidget->addWidget(contents);
}

void CalibrationDialog::onTraitCalibration(const mccmsg::TmCalibrationPtr& msg)
{
    for (int i = 0; i < _contentsWidget->count(); ++i)
    {
        auto page = dynamic_cast<CalibrationDialogPage*>(_contentsWidget->widget(i));
        if (page)
            page->controller()->onTraitCalibrationState(msg);

        auto page1 = dynamic_cast<SimpleFlightModesWidget*>(_contentsWidget->widget(i));
        if (page1)
            page1->onCalibrationState(msg);
    }
}

void CalibrationDialog::onCommonCalibrationStatus(const mccmsg::TmCommonCalibrationStatusPtr& msg)
{
    for (int i = 0; i < _contentsWidget->count(); ++i)
    {
        auto page = dynamic_cast<CalibrationDialogPage*>(_contentsWidget->widget(i));

        if (!page)
            continue;
        page->controller()->onCommonCalibrationStatus(msg);
    }
}

void CalibrationDialog::selectionChanged(mccuav::Uav* uav)
{
    if (_device.isSome())
        return;

    _uav = uav;

    if(!uav)
        return;

    for (int i = 0; i < _contentsWidget->count(); ++i)
    {
        auto page = dynamic_cast<CalibrationDialogPage*>(_contentsWidget->widget(i));

        if (page)
            page->setDevice(uav->device());

        auto page1 = dynamic_cast<SimpleFlightModesWidget*>(_contentsWidget->widget(i));
        if (page1)
            page1->setDevice(uav->device());
    }
}

void CalibrationDialog::deviceFirmwareLoaded(mccuav::Uav* uav)
{
    if (_uav == uav)
        selectionChanged(uav);
}

// void CalibrationDialog::setVisible(bool visible)
// {
//     if(visible)
//     {
//         auto device = _uavController->selectedDevice();
//         if (device == nullptr)
//         {
//             return;
//         }
//
//         _sectionsWidget->setCurrentRow(0);
//
//         for (int i = 0; i < _contentsWidget->count(); ++i)
//         {
//             auto page = dynamic_cast<CalibrationDialogPage*>(_contentsWidget->widget(i));
//
//             if(page)
//                 page->setDevice(device->id());
//
//             auto page1 = dynamic_cast<SimpleFlightModesWidget*>(_contentsWidget->widget(i));
//             if(page1)
//                 page1->setDevice(device->id());
//         }
//
//         setWindowTitle(QString("Предполетная подготовка: %1").arg(device->getInfo()));
//     }
//
//     for (int i = 0; i < _contentsWidget->count(); ++i)
//     {
//         auto page = dynamic_cast<CalibrationDialogPage*>(_contentsWidget->widget(i));
//
//         if (page)
//         {
//             page->controller()->setRunning(visible);
//         }
//     }
//    mccui::Dialog::setVisible(visible);
//}

void CalibrationDialog::setStatus(mccmsg::CalibrationSensor sensor, bool status)
{

}

void CalibrationDialog::calibrationStarted(mccmsg::CalibrationSensor sensor)
{

}

void CalibrationDialog::calibrationCancelled(mccmsg::CalibrationSensor sensor)
{

}

void CalibrationDialog::reject()
{
    //mccui::Dialog::reject();
}
}
