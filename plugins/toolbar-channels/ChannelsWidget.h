#pragma once

#include "mcc/Config.h"
#include "mcc/uav/Fwd.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Rc.h"

#include <QWidget>

class QLabel;

class ChannelsListDelegate;
class ChannelsListModel;
class ChannelsListView;
class ChannelsPropertiesDialog;

class ChannelsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChannelsWidget(mccuav::ChannelsController* chanController,
                            mccuav::GlobalActions* actions,
                            QWidget* parent = nullptr);
    ~ChannelsWidget() override;

    bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void updateContent();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    mccui::Rc
    <mccuav::ChannelsController>        _channelsController;
    mccui::Rc<mccuav::GlobalActions>    _actions;

    ChannelsListModel*                  _model;
    ChannelsListView*                   _view;
    ChannelsListDelegate*               _delegate;

    ChannelsPropertiesDialog*           _propertiesDialog;

    mccui::ClickableLabel*              _noChannels;
    QLabel*                             _noActivated;
    mccui::ClickableLabel*              _moreLabel;

    Q_DISABLE_COPY(ChannelsWidget)
};
