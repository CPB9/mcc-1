#include "../widgets/FirmwareWidget.h"

#include <QListWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "../widgets/MavlinkParametersWidget.h"
#include "../widgets/MavlinkMonitorWidget.h"

#include "AirframeWidget.h"
#include "mcc/calib/CalibrationDialog.h"
#include "mcc/uav/UavController.h"

namespace mccmav {

FirmwareWidget::~FirmwareWidget()
{
}

FirmwareWidget::FirmwareWidget(const mccmsg::Protocol& protocol, mccuav::ChannelsController* chanController, mccuav::UavController* uavController)
    : _protocol(protocol)
    , _uavController(uavController)
{
    _sectionsWidget = new QListWidget();
    _sectionsWidget->setViewMode(QListView::ListMode);
    _sectionsWidget->setIconSize(QSize(96, 84));
    _sectionsWidget->setMovement(QListView::Static);
    _sectionsWidget->setMaximumWidth(128);
    _sectionsWidget->setSpacing(12);

    _contentsWidget = new QStackedWidget();

    auto horizontalLayout = new QHBoxLayout();
    horizontalLayout->addWidget(_sectionsWidget);
    horizontalLayout->addWidget(_contentsWidget, 1);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    connect(_sectionsWidget, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem* current, QListWidgetItem* previous)
            {
                if (current == previous)
                    return;
                _contentsWidget->setCurrentIndex(_sectionsWidget->row(current));
            }
    );

    setLayout(horizontalLayout);

    auto airframe = new AirframeWidget(protocol, uavController, this);
    auto params = new MavlinkParametersWidget(chanController, uavController, this);
    //auto monitor = new MavlinkMonitorWidget(uavController, this);

    addPage("Airframe", airframe);
    addPage("Параметры", params);
    //addPage("Монитор", monitor);

    auto dialog = new mcccalib::CalibrationDialog(uavController, this);

    addPage("Калибровка", dialog);

    connect(_uavController.get(), &mccuav::UavController::traitCalibration,
            [dialog](const mccmsg::TmCalibrationPtr& msg)
            {
                dialog->onTraitCalibration(msg);
            }
    );
    connect(_uavController.get(), &mccuav::UavController::traitCommonCalibrationStatus,
            [dialog](const mccmsg::TmCommonCalibrationStatusPtr& msg)
            {
                dialog->onCommonCalibrationStatus(msg);
            }
    );

    connect(_uavController.get(), &mccuav::UavController::selectionChanged, this,
            [params, this](mccuav::Uav* uav)
            {
                if (!uav)
                    return;

                if (uav->protocol() != _protocol)
                    return;
                params->selectionChanged(uav);
            }
    );

//     connect(_uavController.get(), &mccuav::UavController::uavFirmwareLoaded, this,
//             [params, this](mccuav::Uav* uav)
//             {
//                 if (!uav)
//                     return;
//                 if (uav->protocol() != _protocol)
//                     return;
//                 params->firmwareLoaded(uav);
//             }
//     );

    connect(_uavController.get(), &mccuav::UavController::uavTmStorageUpdated, this,
            [params, this](mccuav::Uav* uav)
            {
                if (!uav)
                    return;
                if (uav->protocol() != _protocol)
                    return;
                params->firmwareLoaded(uav);
            }
    );
}

void FirmwareWidget::addPage(const QString& section, QWidget* contents)
{
    QListWidgetItem *configButton = new QListWidgetItem(_sectionsWidget);
    configButton->setText(section);
    configButton->setTextAlignment(Qt::AlignLeft);
    configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    _contentsWidget->addWidget(contents);
}
}
