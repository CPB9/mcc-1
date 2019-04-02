#include "ChannelsPropertiesWidget.h"

#include "mcc/ide/view/NetStatisticsWidget.h"
#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/ui/ClickableLabel.h"
#include "mcc/ui/SliderCheckBox.h"
#include "mcc/ui/TextUtils.h"

#include <bmcl/Uuid.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>

ChannelsPropertiesWidget::ChannelsPropertiesWidget(mccuav::ChannelsController* channelsController,
                                                   const mccmsg::Channel& channel,
                                                   QWidget *parent)
    : QWidget(parent)
    , _iconsSize(14, 14)
    , _channelsController(channelsController)
    , _channel(bmcl::Uuid::createNil())
    , _protocolIcon(new QLabel(this))
    , _name(new QLabel("Channel", this))
    , _statistics(new mccide::NetStatisticsWidget(this))
    , _menu(new mccui::ClickableLabel(QPixmap::fromImage(QImage(":/toolbar/main_menu_passive.png").scaled(_iconsSize)),
                                      QPixmap::fromImage(QImage(":/toolbar/main_menu_active.png").scaled(_iconsSize)),
                                      this))
    , _slider(new mccui::OnOffSliderCheckBox(true, this))
{
    int h = 34;
    _statistics->setFixedSize(h * 2, h);
    _statistics->setFocusPolicy(Qt::ClickFocus);
    _statistics->setStyleSheet("border: 0px; "
                               "background-color: rgba(0, 0, 0, 255);"
                               "color: white;");
    _statistics->setTextColor(Qt::white);

    _menu->setMaximumSize(_iconsSize);
    _menu->setToolTip("Открыть диалог настроек канала обмена");
    _slider->setToolTip("Включение/отключение режима быстрой регистрации");

    QVBoxLayout* techLayout = new QVBoxLayout(this);
    techLayout->setContentsMargins(8, 0, 8, 0);
    techLayout->setSpacing(0);

    QHBoxLayout* mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(_protocolIcon);
    mainLayout->addWidget(_name);
    mainLayout->addStretch();
    mainLayout->addWidget(_statistics);

    QVBoxLayout* actionsLayout = new QVBoxLayout();
    mainLayout->addLayout(actionsLayout);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->setSpacing(2);
    actionsLayout->addStretch(1);
    actionsLayout->addWidget(_menu, 1, Qt::AlignRight);
    actionsLayout->addWidget(_slider);
    actionsLayout->addStretch(1);

    _protocolIcon->setStyleSheet("background-color: transparent;");
    _name->setStyleSheet("background-color: transparent;");
    _menu->setStyleSheet("background-color: transparent;");

    techLayout->addLayout(mainLayout);
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    techLayout->addWidget(line);

    connect(_slider, &mccui::OnOffSliderCheckBox::sliderStateChanged,
            this, [this](const bmcl::Uuid&, bool checked)
    {
        if(!_channel.isNil())
            _channelsController->requestChannelActivate(_channel, checked, parentWidget());
    });
    connect(_menu, &mccui::ClickableLabel::clicked,
            this, [this]()
    {
        if(!_channel.isNil())
            emit channelMenuClicked(_channel);
    });

    setChannel(channel);
}

ChannelsPropertiesWidget::~ChannelsPropertiesWidget()
{}

void ChannelsPropertiesWidget::setChannel(const mccmsg::Channel& channel)
{
    if(_channel == channel)
        return;

    _channel = channel;

    updateName();
    updateActivation();
}

void ChannelsPropertiesWidget::updateName()
{
    if(_channel.isNil())
        return;

    auto info = _channelsController->channelInformation(_channel);
    if(info.isNone())
        return;

    if(info->channelDescription().isSome())
        _name->setText(mccui::shortTextLine(QString::fromStdString(info->channelDescription()->info()), 16));
}

void ChannelsPropertiesWidget::updateActivation()
{
    if(_channel.isNil())
        return;

    auto info = _channelsController->channelInformation(_channel);
    if(info.isNone())
        return;

    _statistics->setVisible(info->isActive());
    _slider->setChecked(info->isActive());
}

void ChannelsPropertiesWidget::setProtocolIcon(const QPixmap& pixmap)
{
    if(_protocolIcon->pixmap() != nullptr && pixmap.cacheKey() == _protocolIcon->pixmap()->cacheKey())
        return;

    _protocolIcon->setPixmap(pixmap);
}

void ChannelsPropertiesWidget::setStatistics(const mccmsg::StatChannel& stat)
{
    _statistics->updateStats(stat._sent, stat._rcvd, stat._bad);
}

void ChannelsPropertiesWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing, true);

//    int lineY = size().height() - 1;
//    constexpr int marg(6);
//    painter.setPen(QColor(160, 160, 160, 160));
//    painter.drawLine(marg, lineY, size().width() - marg, lineY);
}
