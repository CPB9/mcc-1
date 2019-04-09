#include "UavsDialog.h"
#include "ToolbarUavSettingsPage.h"
#include "UavWidget.h"

#include "mcc/ide/toolbar/AddEntityWidget.h"
#include "mcc/ide/toolbar/MainToolBar.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/uav/UavController.h"
#include "mcc/ui/ClickableLabel.h"
#include "mcc/res/Resource.h"

#include <QEvent>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QScrollArea>
#include <QVBoxLayout>

UavsDialog::UavsDialog(mccuav::UavController* uavController,
                       mccuav::GlobalActions* actions,
                       ToolbarUavSettingsPage* settings,
                       QWidget* parent)
    : mccui::Dialog (parent)
    , _uavController(uavController)
    , _actions(actions)
    , _settings(settings)
    , _uavWidgets()
    , _addUavWidget(new mccide::AddEntityWidget(QImage(":/toolbar-selecteddevice/resources/add_passive.png"),
                                                QImage(":/toolbar-selecteddevice/resources/add_active.png")))
    , _view(new QScrollArea(this))
    , _uavMenu(new QMenu(this))
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

    QWidget* viewport = new QWidget();
    QVBoxLayout* vL = new QVBoxLayout(viewport);
    vL->setContentsMargins(0, 0, 0, 0);
    vL->setSpacing(0);
    viewport->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    _view->setWidget(viewport);
    _view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(_uavController.get(), &mccuav::UavController::uavDescriptionChanged,
            this, &UavsDialog::updateDeviceDescription);

    connect(_uavController.get(), &mccuav::UavController::uavAdded, this, &UavsDialog::updateList);
    connect(_uavController.get(), &mccuav::UavController::uavRemoved, this, &UavsDialog::updateList);
    connect(_uavController.get(), &mccuav::UavController::uavListReordered, this, &UavsDialog::updateList);

    _view->widget()->layout()->addWidget(_addUavWidget);
    connect(_addUavWidget, &mccide::AddEntityWidget::clicked,
            this, [this]()
    {
        _actions->showAddUavDialog();
    });

    updateList();

    QAction* editAction = new QAction(mccres::loadIcon(mccres::ResourceKind::EditButtonIcon), "Редактировать", this);
    QAction* removeAction = new QAction(mccres::loadIcon(mccres::ResourceKind::DeleteButtonIcon), "Удалить", this);
    _uavMenu->addAction(editAction);
    _uavMenu->addAction(removeAction);

    connect(editAction, &QAction::triggered, this,
            [this]()
    {
        _actions->showEditUavDialog(_menuDevice);
    });
    connect(removeAction, &QAction::triggered, this,
            [this]()
    {
        auto device = _uavController->uav(_menuDevice);
        if(device.isNone())
            return;
        _actions->showRemoveUavDialog(device->device());
    });

    connect(_settings, &ToolbarUavSettingsPage::showListUavStatisticsChanged, this,
            [this](bool show)
    {
        for(auto w : _uavWidgets)
            w->setStatisticsVisible(show);
    });

    setStyleSheet(QString(
        "QWidget\n"
        "{\n"
        "	background-color: #%1;\n"
        "}\n\n"
    ).arg(mccide::MainToolBar::mainBackgroundColor().rgb(), 6, 16, QLatin1Char('0')));
}

UavsDialog::~UavsDialog()
{}

bool UavsDialog::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        for(auto w : _uavWidgets)
        {
            if(watched == w)
            {
                if(w->currentUav() != _uavController->selectedUav())
                {
                    _uavController->selectUav(w->currentUav());

                    return true;
                }
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void UavsDialog::updateDeviceDescription(const mccuav::Uav* vehicle)
{
    for(auto& widget : _uavWidgets)
    {
        if(widget->currentUav() == vehicle)
        {
            widget->updateName();
            return;
        }
    }
}

void UavsDialog::updateList()
{
    // add new
    for(auto device : _uavController->uavsList())
    {
        bool found = std::any_of(_uavWidgets.begin(), _uavWidgets.end(), [&](const auto& w) { return w->currentUav() == device; });

        if(!found)
        {
            UavWidget* w = new UavWidget(device, _uavController.get(), _actions.get());
            w->setStatisticsVisible(_settings->showListUavStatisticsState());
            w->setMinimumHeight(mccide::MainToolBar::blockMinimumSize().height());
            w->installEventFilter(this);

            connect(w, &UavWidget::vehicleMenuClicked, this, &UavsDialog::execVehicleMenu);

            _uavWidgets.push_back(w);
        }
    }

    //remove old
    for(auto it = _uavWidgets.begin(); it != _uavWidgets.end();)
    {
        const auto& devices = _uavController->uavsList();
        bool has = std::any_of(devices.begin(), devices.end(), [&](const auto d) { return d == (*it)->currentUav(); });
        if(!has)
        {
            UavWidget* w = *it;
            it = _uavWidgets.erase(it);
            delete w;
        }
        else
            ++it;
    }

    std::stable_sort(_uavWidgets.begin(), _uavWidgets.end(),
                     [](UavWidget* left, UavWidget* right)
    {
        return QString::compare(left->currentUav()->getName(),
                                right->currentUav()->getName(),
                                Qt::CaseInsensitive) < 0;
    });

    int row(0);
    for(auto w : _uavWidgets)
    {
        static_cast<QVBoxLayout*>(_view->widget()->layout())->insertWidget(row, w);
        ++row;
    }
}

void UavsDialog::setAddVehicleVisible(bool visible)
{
    _addUavWidget->setVisible(visible);
}

bool UavsDialog::isAddVehicleVisible() const
{
    return _addUavWidget->isVisible();
}

void UavsDialog::execVehicleMenu(const mccmsg::Device device)
{
    _menuDevice = device;
    _uavMenu->exec(QCursor::pos());
}

void UavsDialog::showEvent(QShowEvent* event)
{
    int width(0);
    int height(mccide::MainToolBar::blockMinimumSize().height());
    for(const auto w : _uavWidgets)
    {
        height += mccide::MainToolBar::blockMinimumSize().height() + 3;
        if(w->minimumWidth() > width)
            width = w->minimumWidth();
    }
    resize(width, height + 2);

    _view->widget()->setFixedSize(width, height);
    _view->setFixedSize(width, height + 2);

    mccui::Dialog::showEvent(event);
}
