#include "ChannelsWidget.h"

#include "ChannelsListDelegate.h"
#include "ChannelsListModel.h"
#include "ChannelsListView.h"
#include "ChannelsPropertiesDialog.h"
#include "mcc/ide/toolbar/MainToolBar.h"
#include "mcc/uav/ChannelsController.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/ui/ClickableLabel.h"

#include <QFontMetrics>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>

constexpr int iconSize = 32;

ChannelsWidget::ChannelsWidget(mccuav::ChannelsController* chanController,
                               mccuav::GlobalActions* actions,
                               QWidget* parent)
    : QWidget (parent)
    , _channelsController(chanController)
    , _actions(actions)
    , _model(new ChannelsListModel(chanController, this))
    , _view(new ChannelsListView(this))
    , _delegate(new ChannelsListDelegate(this))
    , _propertiesDialog(new ChannelsPropertiesDialog(chanController, actions, this))
    , _noChannels(new mccui::ClickableLabel(QPixmap::fromImage(QImage(":/toolbar-channels/resources/add_passive.png").
                                                               scaled(QSize(iconSize, iconSize), Qt::KeepAspectRatio, Qt::SmoothTransformation)),
                                            QPixmap::fromImage(QImage(":/toolbar-channels/resources/add_active.png").
                                                               scaled(QSize(iconSize, iconSize), Qt::KeepAspectRatio, Qt::SmoothTransformation))))
    , _noActivated(new QLabel("Связь\nотключена"))
    , _moreLabel(new mccui::ClickableLabel(QPixmap(":/toolbar-channels/resources/more_passive.png"),
                                           QPixmap(":/toolbar-channels/resources/more_active.png")))
{
    QHBoxLayout* mL = new QHBoxLayout(this);
    mL->setContentsMargins(0, 0, 0, 0);
    mL->setSpacing(0);
    setLayout(mL);

    int maxH = mccide::MainToolBar::blockMinimumSize().height();

    mL->addWidget(_view);
    _view->setModel(_model);
    _view->setItemDelegate(_delegate);
    _view->setMaximumHeight(maxH);
    _view->setMaximumWidth(maxH * 4);
    _view->installEventFilter(this);

    mL->addWidget(_noChannels);
    _noChannels->setFixedSize(maxH, maxH);
    _noChannels->installEventFilter(this);
    _noChannels->setToolTip("Добавить новый канал обмена");
    _noChannels->setAlignment(Qt::AlignCenter);

    mL->addWidget(_noActivated);
    _noActivated->setAlignment(Qt::AlignCenter);
    _noActivated->setMargin(10);
    _noActivated->setMaximumHeight(maxH);
    _noActivated->installEventFilter(this);
    _noActivated->setToolTip("Открыть список каналов");

    mL->addWidget(_moreLabel);
    _moreLabel->installEventFilter(this);

    connect(_model, &ChannelsListModel::modelReset, this, &ChannelsWidget::updateContent);

    QFontMetrics fm(font());
    _noActivated->setMinimumWidth(fm.boundingRect(_noActivated->text()).width());

    updateContent();

    _propertiesDialog->hide();
}

ChannelsWidget::~ChannelsWidget()
{}

bool ChannelsWidget::eventFilter(QObject* watched, QEvent* event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        if(watched == _noChannels)
        {
            _actions->showAddChannelDialog();
        }
        else
        {
            _propertiesDialog->show();
            _propertiesDialog->move(mapToGlobal(QPoint(0, height())));
        }

        event->accept();
        return true;
    }

    return QWidget::eventFilter(watched, event);
}

void ChannelsWidget::updateContent()
{
    bool noActive = !_model->rowCount() && _channelsController->channelsCount() > 0;

    _view->setMaximumWidth(_model->rowCount() * _delegate->defaulSize().width() + 2);
    _view->setVisible(_model->rowCount());
    _noChannels->setVisible(_channelsController->channelsCount() == 0);
    _noActivated->setVisible(noActive);
    _moreLabel->setVisible(_model->hasMoreChannels());

    if(_channelsController->channelsCount() == 0)
        setMinimumWidth(_noChannels->minimumWidth());
    else if(noActive)
        setMinimumWidth(_noActivated->minimumWidth());
    else
        setMinimumWidth(0);

    adjustSize();
}

void ChannelsWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
}

void ChannelsWidget::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
}
