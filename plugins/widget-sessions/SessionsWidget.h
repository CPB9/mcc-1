#pragma once

#include <QWidget>
#include <QTime>

#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Settings.h"
#include "mcc/uav/ExchangeService.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/ChannelsController.h"

#include "mcc/msg/FwdExt.h"

class QToolBar;
class QTableView;
class QAction;
class QPushButton;
class QLabel;

class SessionsModel;

class SessionTool;

class SessionsWidget : public QWidget
{
    Q_OBJECT

public:
    SessionsWidget(const mccui::Rc<mccui::Settings>& settings, 
                   const mccui::Rc<mccuav::ExchangeService>& exchangeService,
                   const mccui::Rc<mccuav::UavController>& uavController,
                   const mccui::Rc<mccuav::ChannelsController>& channelsController);
    ~SessionsWidget();

private slots:
    void onTmSessionUpdated(const mccmsg::TmSessionDescription&);
    void onTmSessionRegistered(const mccmsg::TmSession& session, bool isRegistered);

    void startNewSession();
    void removeSelectedSession();

    void currentRowChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    void setStartedIcon(bool isStarted);

    QTableView* _view;
    SessionsModel* _model;
    QToolBar* _toolbar;
    QAction* _removeSessionAction;
    QLabel* _recordLabel;
    QLabel* _sessionName;
    QLabel* _sessionTime;
    QTime  _time;

    bmcl::Option<mccmsg::TmSessionDescription> _currentSession;
    QPushButton* _playButton;
    QPushButton* _detailsButton;

    SessionTool* _sessionTool;
    mccui::Rc<mccui::Settings>            _settings;
    mccui::Rc<mccuav::ExchangeService>    _exchangeService;
    mccui::Rc<mccuav::UavController>      _uavController;
    mccui::Rc<mccuav::ChannelsController> _channelsController;
protected:
    virtual void mousePressEvent(QMouseEvent *event) override;


    virtual void timerEvent(QTimerEvent *event) override;

};

