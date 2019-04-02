#include "UavWidget.h"
#include "UavFailureWidget.h"
#include "UavModeWidget.h"
#include "UavNameWidget.h"
#include "UavStatisticsWidget.h"

#include "mcc/ide/toolbar/MainToolBar.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/uav/Uav.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/UavExecCommands.h"
#include "mcc/ui/ClickableLabel.h"
#include "mcc/ui/SliderCheckBox.h"
#include "mcc/ui/TextUtils.h"
#include "mcc/msg/exts/StateStorage.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>

constexpr int layoutMargin = 6;
constexpr int actionsMargin = 4;

UavWidget::UavWidget(mccuav::Uav* uav,
                     mccuav::UavController* uavController,
                     mccuav::GlobalActions* actions,
                     QWidget* parent)
    : QWidget (parent)
    , _iconsSize(14, 14)
    , _uavController(uavController)
    , _actions(actions)
    , _currentUav(nullptr)
    , _nameWidget(new UavNameWidget(this))
    , _modeWidget(new UavModeWidget(this))
    , _failureWidget(new UavFailureWidget(this))
    , _statisticsWidget(new UavStatisticsWidget(this))
    , _menu(new mccui::ClickableLabel(QPixmap::fromImage(QImage(":/toolbar/main_menu_passive.png").scaled(_iconsSize)),
                                      QPixmap::fromImage(QImage(":/toolbar/main_menu_active.png").scaled(_iconsSize)),
                                      this))
    , _slider(new mccui::OnOffSliderCheckBox(true, this))
    , _line(new QFrame)
    , _separatedMode(false)
    , _hovered(false)
    , _isStatisticsVisible(true)
{
    QVBoxLayout* techLayout = new QVBoxLayout(this);
    techLayout->setContentsMargins(layoutMargin, 0, layoutMargin, 0);
    techLayout->setSpacing(0);

    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(_nameWidget);
    mainLayout->addWidget(_statisticsWidget);
    mainLayout->addWidget(_modeWidget);
    mainLayout->addWidget(_failureWidget);
    mainLayout->addStretch();

    _failureWidget->hide();

    QVBoxLayout* actionsLayout = new QVBoxLayout();
    mainLayout->addLayout(actionsLayout);
    actionsLayout->setContentsMargins(0, 0, actionsMargin, 0);
    actionsLayout->setSpacing(2);
    actionsLayout->addStretch(1);
    actionsLayout->addWidget(_menu, 1, Qt::AlignRight);
    actionsLayout->addWidget(_slider);
    actionsLayout->addStretch(1);
    _menu->setStyleSheet("background-color: transparent;");

    _menu->setToolTip("Открыть диалог настроек аппарата");
    _slider->setToolTip("Влючение/отключение обмена с аппаратом");

    techLayout->addLayout(mainLayout);
    _line->setFrameShape(QFrame::HLine);
    _line->setFrameShadow(QFrame::Sunken);
    _line->setVisible(!_separatedMode);
    techLayout->addWidget(_line);

    QString ttStyle = QString(
        "QToolTip {"
        "    background: darkgray;"
        "    color: gray;"
        "}"
        );
    setStyleSheet(ttStyle);

    connect(_menu, &mccui::ClickableLabel::clicked,
            this, [this]()
    {
        _hovered = false; // Hack for QEvent::HoverLeave bug
        update();

        if(currentVehicle() != nullptr)
            emit vehicleMenuClicked(currentVehicle()->device());
    });
    connect(_slider, &mccui::OnOffSliderCheckBox::sliderStateChanged,
            this, [this](const bmcl::Uuid&, bool checked)
    {
        _uavController->activateUav(currentVehicle(), checked);
    });
    connect(_uavController.get(), &mccuav::UavController::selectionChanged, this, &UavWidget::checkSelection);

    updateMinimumSize();
    updateCurrentUav(uav);
}

UavWidget::~UavWidget()
{}

void UavWidget::tmStorageClear()
{
    if (_currentUav == nullptr || _currentUav->tmStorage().isNull())
        return;
    const auto& tm = _currentUav->tmStorage();
    tm->removeHandler(_handlerState);
    _handlerState.clear();
}

void UavWidget::tmStorageUpdated()
{
    if (_currentUav == nullptr || _currentUav->tmStorage().isNull())
        return;
    const auto& tm = _currentUav->tmStorage();

    auto s = tm->getExtension<mccmsg::IStateStorage>();
    if (s.isSome())
    {
        _handlerState = s->addHandler([this] { updateModes(); updateFailures(); }, true);
    }
}

void UavWidget::updateCurrentUav(mccuav::Uav* uav)
{
    if(currentVehicle() == uav)
        return;

    if(currentVehicle() != nullptr)
    {
        currentVehicle()->disconnect(this);
        currentVehicle()->execCommands()->disconnect(this);
        tmStorageClear();
    }

    _currentUav = uav;

    if(_currentUav == nullptr)
        return;

    connect(currentVehicle(), &mccuav::Uav::pixmapChanged, this, &UavWidget::updatePixmap);
    connect(currentVehicle(), &mccuav::Uav::activatedChanged, this, &UavWidget::updateVehicleActivation);
    connect(currentVehicle()->execCommands(), &mccuav::UavExecCommands::commandsChanged, this, &UavWidget::updateCommands);
    connect(currentVehicle(), &mccuav::Uav::uavStatisticsChanged, this, &UavWidget::updateStatistics);
    connect(currentVehicle(), &mccuav::Uav::tmStorageUpdated, this, &UavWidget::tmStorageUpdated);

    tmStorageUpdated();
    updateVehicleActivation();

    updateCommands();
    updateModes();

    checkSelection();
}

void UavWidget::updateVehicleActivation()
{
    if(currentVehicle() == nullptr)
        return;

    _slider->setChecked(currentVehicle()->isActivated());

    if(!currentVehicle()->isActivated())
        _nameWidget->activateProcess(false);

    _modeWidget->setVisible(currentVehicle()->isActivated() && _modeWidget->mayToShow());
    _failureWidget->setVisible(currentVehicle()->isActivated() && _failureWidget->mayToShow());
    _statisticsWidget->setVisible(currentVehicle()->isActivated() && _statisticsWidget->mayToShow() && isStatisticsVisible());

    updateMinimumSize();
    updatePixmap();
    updateName();
}

void UavWidget::updatePixmap()
{
    if(currentVehicle() == nullptr)
        return;

    // FIXME: HDPI pixmaps
    constexpr QSize size(32, 32);
    QPixmap pixmap = currentVehicle()->pixmapGenerator().generate(currentVehicle()->color(), size.width(), size.height());
    if(!currentVehicle()->isActivated())
    {
        QImage img(pixmap.size(), QImage::Format_ARGB32_Premultiplied);
        img.fill(QColor(0,0,0,0));
        QPainter painter(&img);
        painter.setOpacity(0.35);
        painter.drawPixmap(0, 0, pixmap);
        pixmap = QPixmap::fromImage(img);
    }

    _nameWidget->setPixmap(pixmap);
}

void UavWidget::updateName()
{
    if(currentVehicle() == nullptr)
        return;

    _nameWidget->setName(currentVehicle()->getName());
}

void UavWidget::updateCommands()
{
    if(currentVehicle() == nullptr)
        return;

    _nameWidget->activateProcess(!currentVehicle()->execCommands()->commands().empty());

    if(!currentVehicle()->execCommands()->commands().empty())
    {
        int value(0);
        for(const auto& command : currentVehicle()->execCommands()->commands())
        {
            value += command.progress();
        }
        value /= currentVehicle()->execCommands()->commands().size();
    }
}

void UavWidget::updateModes()
{
    if(currentVehicle() == nullptr)
        return;

    const auto& s = currentVehicle()->tmStorage();
    if (!s.isNull())
    {
        const auto st = s->getExtension<mccmsg::IStateStorage>();
        if (st.isSome())
        {
            _modeWidget->setMode(st->stateStr());
            _modeWidget->setSubmode(st->subStateStr());
        }
    }
    else
    {
        _modeWidget->setMode(QString());
        _modeWidget->setSubmode(QString());
    }

    _modeWidget->setVisible(currentVehicle()->isActivated() && _modeWidget->mayToShow());
    updateMinimumSize();
}

void UavWidget::updateFailures()
{
    if(currentVehicle() == nullptr)
        return;
    _failureWidget->setVisible(currentVehicle()->isActivated() && _failureWidget->mayToShow());
    updateMinimumSize();
}

void UavWidget::updateStatistics()
{
    if(currentVehicle() == nullptr)
        return;

    _statisticsWidget->setStatistics(currentVehicle()->statDevice());
}

bool UavWidget::event(QEvent* event)
{
    if(event->type() == QEvent::HoverEnter)
    {
        _hovered = true;
        update();
    }
    else if(event->type() == QEvent::HoverLeave)
    {
        _hovered = false;
        update();
    }

    return QWidget::event(event);
}

void UavWidget::paintEvent(QPaintEvent* event)
{
    bool currentInList = _uavController->isUavSelected(currentVehicle()) && !isSeparatedMode();
    if(_hovered || currentInList)
    {
        QColor c;

        if(_hovered)
        {
            if(currentInList)
                c = QColor(0, 0, 40, 255);
            else
                c = mccide::MainToolBar::hoveredBackgroundColor();
        }
        else
            c = QColor(0, 0, 100, 170);

        QPainter painter(this);

        QRect r = rect();
        painter.setPen(c);
        painter.setBrush(c);
        if(!isSeparatedMode())
            r.setHeight(r.height() - _line->height() - 3);
        painter.drawRect(r);
    }

    QWidget::paintEvent(event);
}

void UavWidget::checkSelection()
{
    update();
}

void UavWidget::updateMinimumSize()
{
    int minW(0);

    if(isSeparatedMode())
    {
        if(currentVehicle() == nullptr)
            minW = 0;
        else
            minW = layoutMargin * 2 +
                   _nameWidget->width() +
                   (currentVehicle()->isActivated() && _statisticsWidget->mayToShow() && isStatisticsVisible() ? _statisticsWidget->minimumWidth() : 0) +
                   (currentVehicle()->isActivated() && _modeWidget->mayToShow() ? _modeWidget->minimumWidth() : 0) +
                   (currentVehicle()->isActivated() && _failureWidget->mayToShow() ?  _failureWidget->minimumWidth() : 0) +
                   _slider->minimumWidth() + actionsMargin;
    }
    else
    {
        minW = layoutMargin * 2 +
               _nameWidget->width() +
               (isStatisticsVisible() ? _statisticsWidget->minimumWidth() : 0) +
               _modeWidget->minimumWidth() +
               _failureWidget->minimumWidth() +
               _slider->minimumWidth() + actionsMargin;
    }
    setMinimumWidth(minW);
}

void UavWidget::setSeparatedMode(bool separatedMode)
{
    if(_separatedMode == separatedMode)
        return;

    _separatedMode = separatedMode;

    _line->setVisible(!_separatedMode);

    updateMinimumSize();
    checkSelection();
}

void UavWidget::setStatisticsVisible(bool visible)
{
    if(_isStatisticsVisible == visible)
        return;

    _isStatisticsVisible = visible;

    _statisticsWidget->setVisible(currentVehicle() != nullptr &&
                                  currentVehicle()->isActivated() &&
                                  _statisticsWidget->mayToShow() &&
                                  isStatisticsVisible());
    updateMinimumSize();
    update();
}
