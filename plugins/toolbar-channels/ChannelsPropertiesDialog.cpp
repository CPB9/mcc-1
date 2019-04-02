#include "ChannelsPropertiesDialog.h"
#include "ChannelsPropertiesWidget.h"

#include "mcc/ide/toolbar/AddEntityWidget.h"
#include "mcc/ide/toolbar/MainToolBar.h"// FIXME: temp
#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/res/Resource.h"

#include <QEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollArea>
#include <QVBoxLayout>

ChannelsPropertiesDialog::ChannelsPropertiesDialog(mccuav::ChannelsController* channelsController,
                                                   mccuav::GlobalActions* actions,
                                                   QWidget* parent)
    : mccui::Dialog (parent)
    , _channelsController(channelsController)
    , _actions(actions)
    , _channelWidgets()
    , _protocolIcons()
    , _addChannelWidget(new mccide::AddEntityWidget(QImage(":/toolbar-channels/resources/add_passive.png"),
                                                    QImage(":/toolbar-channels/resources/add_active.png")))
    , _view(new QScrollArea(this))
    , _channelMenu(new QMenu(this))
    , _menuChannel(bmcl::Uuid::createNil())
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    QWidget* viewport = new QWidget();
    QVBoxLayout* vL = new QVBoxLayout(viewport);
    vL->setContentsMargins(0, 0, 0, 4);
    vL->setSpacing(0);
    viewport->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    _view->setWidget(viewport);
    _view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(_channelsController.get(), &mccuav::ChannelsController::protocolsChanged,
            this, &ChannelsPropertiesDialog::updateProtocolsIcons);

    connect(_channelsController.get(), &mccuav::ChannelsController::channelsChanged,
            this, &ChannelsPropertiesDialog::updateChannels);
    connect(_channelsController.get(), &mccuav::ChannelsController::channelDescription,
            this, &ChannelsPropertiesDialog::updateDescription);
    connect(_channelsController.get(), &mccuav::ChannelsController::channelActiveChanged,
            this, &ChannelsPropertiesDialog::updateChannelActivation);
    connect(_channelsController.get(), &mccuav::ChannelsController::channelStatsUpdated,
            this, &ChannelsPropertiesDialog::updateChannelStats);

    _view->widget()->layout()->addWidget(_addChannelWidget);
    connect(_addChannelWidget, &mccide::AddEntityWidget::clicked,
            this, [this]()
    {
        _actions->showAddChannelDialog();
    });

    updateProtocolsIcons();
    updateChannels();

    QAction* editAction = new QAction(mccres::loadIcon(mccres::ResourceKind::EditButtonIcon), "Редактировать", this);
    QAction* removeAction = new QAction(mccres::loadIcon(mccres::ResourceKind::DeleteButtonIcon), "Удалить", this);
    _channelMenu->addAction(editAction);
    _channelMenu->addAction(removeAction);

    connect(editAction, &QAction::triggered, this,
            [this]()
    {
        _actions->showChannelEditDialog(_menuChannel);
    });
    connect(removeAction, &QAction::triggered, this,
            [this]()
    {
        if (QMessageBox::question(nullptr, "Удаление канала",
            "Действительно удалить данный канал связи?",
            QMessageBox::Yes | QMessageBox::No)
            == QMessageBox::Yes)
        {
            _channelsController->requestChannelUnregister(_menuChannel, this);
        }
    });

    setStyleSheet(QString(
        "QWidget\n"
        "{\n"
        "	background-color: #%1;\n"
        "}\n\n"
    ).arg(mccide::MainToolBar::mainBackgroundColor().rgb(), 6, 16, QLatin1Char('0')));
}

ChannelsPropertiesDialog::~ChannelsPropertiesDialog()
{}

void ChannelsPropertiesDialog::updateChannels()
{
    // add new
    int row(0);
    for(const auto& chanInfo : _channelsController->channelInformations())
    {
        bool found = std::any_of(_channelWidgets.begin(), _channelWidgets.end(), [&](const auto& i) { return i->channel() == chanInfo.channel(); });
        if(!found)
        {
            ChannelsPropertiesWidget* c = new ChannelsPropertiesWidget(_channelsController.get(), chanInfo.channel());
            c->setMinimumHeight(mccide::MainToolBar::blockMinimumSize().height());

            if(chanInfo.channelDescription().isSome())
                updateChannelIcon(c, chanInfo.channelDescription().unwrap());

            connect(c, &ChannelsPropertiesWidget::channelMenuClicked,
                    this, [this](const mccmsg::Channel& channel)
            {
                _menuChannel = channel;
                _channelMenu->exec(QCursor::pos());
            });

    //        c->installEventFilter(this);
            static_cast<QVBoxLayout*>(_view->widget()->layout())->insertWidget(row, c);

            _channelWidgets.push_back(c);
            ++row;
        }
    }
    //remove old
    for(auto it = _channelWidgets.begin(); it != _channelWidgets.end();)
    {
        const auto& cs = _channelsController->channelInformations();
        bool has = std::any_of(cs.begin(), cs.end(), [&](const auto& i) { return i.channel() == (*it)->channel(); });
        if(!has)
        {
            ChannelsPropertiesWidget* w = *it;
            it = _channelWidgets.erase(it);
            delete w;
        }
        else
            ++it;
    }
}

void ChannelsPropertiesDialog::updateProtocolsIcons()
{
    _protocolIcons.clear();

    for(const auto& protoDesc : _channelsController->protocols())
    {
        if(!protoDesc->pixmap().isEmpty())
        {
            _protocolIcons[protoDesc->name()] = QPixmap::fromImage(mccres::renderSvg(protoDesc->pixmap().asBytes(), 32, 32));
        }
    }

    for(auto w : _channelWidgets)
    {
        const auto chInfo = _channelsController->channelInformation(w->channel());
        if(chInfo.isSome() && chInfo->channelDescription().isSome())
        {
            updateChannelIcon(w, chInfo->channelDescription().unwrap());
        }
    }
}

void ChannelsPropertiesDialog::updateDescription(const mccmsg::Channel& channel, const mccmsg::ChannelDescription& description)
{
    for(auto c : _channelWidgets)
    {
        if(c->channel() == channel)
        {
            c->updateName();
            updateChannelIcon(c, description);
            return;
        }
    }
}

void ChannelsPropertiesDialog::updateChannelStats(const mccmsg::Channel& channel, const mccmsg::StatChannel& stat)
{
    for(auto c : _channelWidgets)
    {
        if(c->channel() == channel)
        {
            c->setStatistics(stat);
            return;
        }
    }
}

void ChannelsPropertiesDialog::updateChannelActivation(const mccmsg::Channel& channel, bool)
{
    for(auto c : _channelWidgets)
    {
        if(c->channel() == channel)
        {
            c->updateActivation();
            return;
        }
    }
}

void ChannelsPropertiesDialog::showEvent(QShowEvent* event)
{
    int width(0);
    int height(mccide::MainToolBar::blockMinimumSize().height());
    for(const auto w : _channelWidgets)
    {
        height += w->height();
        if(w->width() > width)
            width = w->width();
    }
    width = this->width();
    width = 256; // FIXME: temp hack
    resize(width, height);


    _view->widget()->setFixedSize(width, height);
        _view->setFixedSize(width, height);

    mccui::Dialog::showEvent(event);
}

void ChannelsPropertiesDialog::updateChannelIcon(ChannelsPropertiesWidget* widget, const mccmsg::ChannelDescription& description)
{
    if(widget == nullptr)
        return;

    const auto i = _protocolIcons.find(description->protocol());
    if(i != _protocolIcons.end())
        widget->setProtocolIcon(i->second);
}
