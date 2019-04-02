#include "SelectedUavWidget.h"
#include "ToolbarUavSettingsPage.h"
#include "UavWidget.h"
#include "UavsDialog.h"

#include "mcc/ide/toolbar/MainToolBar.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/uav/Group.h"
#include "mcc/uav/GroupsController.h"
#include "mcc/uav/Uav.h"
#include "mcc/uav/UavController.h"
#include "mcc/ui/ClickableLabel.h"

#include <QComboBox>
#include <QEvent>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>

constexpr int addIconSize = 32;

SelectedUavWidget::SelectedUavWidget(mccuav::GroupsController* groupsController,
                                     mccuav::UavController* uavController,
                                     mccuav::GlobalActions* actions,
                                     ToolbarUavSettingsPage* settings,
                                     QWidget* parent)
    : QWidget (parent)
    , _groupsController(groupsController)
    , _uavController(uavController)
    , _actions(actions)
    , _settings(settings)
    , _noUavsLabel(new mccui::ClickableLabel(QPixmap::fromImage(QImage(":/toolbar-selecteddevice/resources/add_passive.png").
                                                                scaled(QSize(addIconSize, addIconSize), Qt::KeepAspectRatio, Qt::SmoothTransformation)),
                                             QPixmap::fromImage(QImage(":/toolbar-selecteddevice/resources/add_active.png").
                                                                scaled(QSize(addIconSize, addIconSize), Qt::KeepAspectRatio, Qt::SmoothTransformation))))
    , _notSelectedLabel(new QLabel("Аппарат\n"
                                   "не выбран"))
    , _currentVehicleWidget(new UavWidget(uavController->selectedUav(), uavController, actions, this))
    , _vehiclesDialog(new UavsDialog(uavController, actions, settings, this))
    , _isAllowAddVehicle(true)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(_noUavsLabel);
    layout->addWidget(_notSelectedLabel);
    layout->addWidget(_currentVehicleWidget);

    _noUavsLabel->installEventFilter(this);
    _noUavsLabel->setToolTip("Добавить новый аппарат");
    _noUavsLabel->setAlignment(Qt::AlignCenter);

    _notSelectedLabel->installEventFilter(this);
    _notSelectedLabel->setAlignment(Qt::AlignCenter);
    _notSelectedLabel->setStyleSheet("padding-left: 12px;"
                                     "padding-right: 12px;");
    _notSelectedLabel->setToolTip("Открыть список аппаратов");
    _notSelectedLabel->hide();

    _currentVehicleWidget->installEventFilter(this);
    _currentVehicleWidget->setSeparatedMode(true);

    connect(_uavController.get(), &mccuav::UavController::uavDescriptionChanged, this, &SelectedUavWidget::updateDeviceDescription);
    connect(_uavController.get(), &mccuav::UavController::selectionChanged, this, &SelectedUavWidget::updateSelection);

    _vehiclesDialog->hide();

    connect(_currentVehicleWidget, &UavWidget::vehicleMenuClicked, _vehiclesDialog, &UavsDialog::execVehicleMenu);

    connect(_settings, &ToolbarUavSettingsPage::showToolbarUavStatisticsChanged, this,
            [this](bool show)
    {
        _currentVehicleWidget->setStatisticsVisible(show);
        updateMinimumWidth();
    });

    QFontMetrics fm(font());
    _notSelectedLabel->setMinimumWidth(fm.boundingRect(_notSelectedLabel->text()).width());

    updateSelection();
}

SelectedUavWidget::~SelectedUavWidget()
{}

bool SelectedUavWidget::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        if(watched == _currentVehicleWidget ||
           watched == _notSelectedLabel)
        {
            if(_uavController->uavsCount() > 0 ||
               _isAllowAddVehicle)
            {
                _vehiclesDialog->move(mapToGlobal(QPoint(0, height())));
                _vehiclesDialog->setAddVehicleVisible(_isAllowAddVehicle);
                _vehiclesDialog->show();

                _currentVehicleWidget->update();

                event->accept();
                return true;
            }
        }
        else if(watched == _noUavsLabel)
        {
            _actions->showAddUavDialog();
            event->accept();
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void SelectedUavWidget::updateDeviceDescription(const mccuav::Uav* vehicle)
{
    if(vehicle == nullptr)
        return;

    if(_currentVehicleWidget->currentVehicle() == vehicle)
        _currentVehicleWidget->updateName();
}

void SelectedUavWidget::updateSelection()
{
    _currentVehicleWidget->updateCurrentUav(_uavController->selectedUav());

    _currentVehicleWidget->setVisible(_uavController->selectedUav() != nullptr);
    _noUavsLabel->setVisible(_uavController->uavsCount() == 0);
    _notSelectedLabel->setVisible(_uavController->selectedUav() == nullptr && _uavController->uavsCount() > 0);

    updateMinimumWidth();
}

void SelectedUavWidget::updateMinimumWidth()
{
    if(_uavController->uavsCount() == 0)
        setMinimumWidth(mccide::MainToolBar::blockMinimumSize().height());
    else if(_uavController->selectedUav() != nullptr)
        setMinimumWidth(_currentVehicleWidget->minimumWidth());
    else
        setMinimumWidth(_notSelectedLabel->minimumWidth());
}
