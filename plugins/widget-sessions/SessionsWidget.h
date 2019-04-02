#pragma once

#include <QWidget>

#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"
#include "mcc/uav/ExchangeService.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/ChannelsController.h"

class QToolBar;
class QTableView;
class QAction;

class SessionsModel;

class SessionsWidget : public QWidget
{
    Q_OBJECT

public:
    SessionsWidget(const mccui::Rc<mccuav::ExchangeService>& exchangeService,
                   const mccui::Rc<mccuav::UavController>& uavController,
                   const mccui::Rc<mccuav::ChannelsController>& channelsController);

private slots:
    void onTmSessionUpdated(const mccmsg::TmSessionDescription&);
    void onTmSessionRegistered(const mccmsg::TmSession& session, bool isRegistered);

    void startNewSession();
    void removeSelectedSession();

    void currentRowChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    QTableView* _view;
    SessionsModel* _model;
    QToolBar* _toolbar;
    QAction* _removeSessionAction;

    mccui::Rc<mccuav::ExchangeService>    _exchangeService;
    mccui::Rc<mccuav::UavController>      _uavController;
    mccui::Rc<mccuav::ChannelsController> _channelsController;
};

