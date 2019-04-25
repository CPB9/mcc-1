#include "SessionsWidget.h"
#include "SessionsModel.h"
#include "SessionTool.h"

#include "mcc/ui/WidgetUtils.h"
#include "mcc/res/Resource.h"
#include "mcc/msg/obj/TmSession.h"
#include "mcc/msg/ptr/TmSession.h"

#include <bmcl/MakeRc.h>
#include <bmcl/TimeUtils.h>

#include <QVBoxLayout>
#include <QTableView>
#include <QToolBar>
#include <QAction>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QInputDialog>
#include <QMessageBox>

SessionsWidget::SessionsWidget(const mccui::Rc<mccui::Settings>& settings,
                               const mccui::Rc<mccuav::ExchangeService>& exchangeService,
                               const mccui::Rc<mccuav::UavController>& uavController,
                               const mccui::Rc<mccuav::ChannelsController>& channelsController)
    : _settings(settings)
    , _exchangeService(exchangeService)
    , _uavController(uavController)
    , _channelsController(channelsController)
{
    setWindowTitle("Управление сессиями");
    setObjectName(windowTitle());
    setWindowIcon(QIcon(":/toolbar-sessions/icon.png"));

//     _toolbar = new QToolBar();
//     auto startSessionAction = _toolbar->addAction("Начать новую сессию");
//     _removeSessionAction = _toolbar->addAction("Удалить сессию");
//     _removeSessionAction->setEnabled(false);
//
//     connect(startSessionAction, &QAction::triggered, this, &SessionsWidget::startNewSession);
//     connect(_removeSessionAction, &QAction::triggered, this, &SessionsWidget::removeSelectedSession);
//
    _playButton = new QPushButton();
    _playButton->setFixedSize(32, 32);
    _playButton->setIconSize(QSize(24, 24));
    _playButton->setToolTip("Начало/останов сессии");

    _detailsButton = new QPushButton();
    _detailsButton->setFixedSize(32, 32);
    _detailsButton->setIconSize(QSize(24, 24));
    _detailsButton->setIcon(QIcon(":/toolbar-sessions/settings.png"));
    _detailsButton->setToolTip("Настройки записи сессии");

    connect(_detailsButton, &QPushButton::pressed, this,
            [this]()
            {
                _sessionTool->show();
                _sessionTool->move(mapToGlobal(QPoint(width() - _sessionTool->width(), height())));
            }
    );
    _sessionName = new QLabel(this);
    _sessionTime = new QLabel(this);
    _recordLabel = new QLabel("REC");
    _recordLabel->setStyleSheet("QLabel{margin-left: 5px; border-radius: 5px; background: red; color: white; font-weight: bold}");
    auto layout = new QGridLayout();
    layout->addWidget(_recordLabel, 0, 0, 2, 1);
    layout->addWidget(_playButton, 0, 1, 2, 1);
    layout->addWidget(_sessionName, 0, 2);
    layout->addWidget(_sessionTime, 1, 2);
    layout->addWidget(_detailsButton, 0, 3, 2, 1);
    setLayout(layout);

    connect(_playButton, &QPushButton::pressed, this, &SessionsWidget::startNewSession);
    _sessionTool = new SessionTool(settings, exchangeService, uavController, channelsController);

//     layout->addWidget(_toolbar);

    _model = new SessionsModel(_uavController, channelsController);
    setStartedIcon(false);
//     _view = new QTableView();
//     _view->setModel(_model);
//
//     layout->addWidget(_view);
//
//     connect(_view->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &SessionsWidget::currentRowChanged);
//
//     exchangeService->requestXX(new mccmsg::tmSession::DescriptionList_Request).then
//     (
//         [this](const mccmsg::tmSession::DescriptionList_ResponsePtr& rep)
//         {
//             for (const auto& r : rep->data())
//             {
//                 _model->updateSession(r);
//             }
//         },
//         [this](const mccmsg::ErrorDscr& err)
//         {
//             BMCL_WARNING() << "Ошибка запроса списка сессий: " << err.full();
//             //QMessageBox::warning(this, "Ошибка при создании канала обмена", "channelRegisterFailed: " + err.qfull());
//         }
//     );
//
     connect(_exchangeService.get(), &mccuav::ExchangeService::tmSessionRegistered, this, &SessionsWidget::onTmSessionRegistered);
     connect(_exchangeService.get(), &mccuav::ExchangeService::tmSessionUpdated, this, &SessionsWidget::onTmSessionUpdated);
     startTimer(1000);
}

SessionsWidget::~SessionsWidget()
{
    delete _sessionTool;
}

void SessionsWidget::onTmSessionUpdated(const mccmsg::TmSessionDescription& session)
{
    _model->updateSession(session);
//     if(_currentSession.isSome() && _currentSession == session->name())
//     {
//         _sessionName->setText(QString::fromStdString(session->info()));
//     }
}

void SessionsWidget::onTmSessionRegistered(const mccmsg::TmSession& session, bool isRegistered)
{
//     if(!isRegistered)
//     {
//         _model->removeSession(session);
//         return;
//     }
//     _exchangeService->requestXX(new mccmsg::tmSession::Description_Request(session)).then
//     (
//         [this](const mccmsg::tmSession::Description_ResponsePtr& rep)
//         {
//             _model->updateSession(rep->data());
//             _currentSession = rep->data();
//             setStartedIcon(true);
//         },
//         [this, &session](const mccmsg::ErrorDscr& err)
//         {
//             BMCL_WARNING() << "Ошибка запроса описания сессии " << session.toStdString() << ": " << err.full();
//         }
//     );

}

void SessionsWidget::startNewSession()
{
    if(_currentSession.isNone())
    {
        bool ok = false;
        QDateTime dt = QDateTime::currentDateTime();
        QString defaultSessionName = QString("%1 %2").arg("Сессия").arg(dt.toString("ddMMyyTHHmmss"));
        auto sessionName = QInputDialog::getText(mccui::findMainWindow(), "Начало новой сессии", "Описание сессии", QLineEdit::Normal, defaultSessionName, &ok);
        if(!ok || sessionName.isEmpty())
            return;

        auto dscr = bmcl::makeRc<const mccmsg::TmSessionDescriptionObj>(mccmsg::TmSession::createNil(), sessionName.toStdString());
        _exchangeService->requestXX(new mccmsg::tmSession::Register_Request(std::move(dscr))).then
        (
            [this](const mccmsg::tmSession::Register_ResponsePtr& rep)
            {
                _model->updateSession(rep->data());
                _currentSession = rep->data();
                _sessionName->setText(QString::fromStdString(rep->data()->info()));
                _time = QTime(0, 0, 0);
                setStartedIcon(true);
            },
            [this](const mccmsg::ErrorDscr& err)
            {
                BMCL_WARNING() << "Ошибка создания новой сессии: " << err.full();
            }
            );
    }
    else
    {
        if(QMessageBox::question(mccui::findMainWindow(), "Управление сессиями", "Закончить сессию?", QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok)
            return;
        mccmsg::TmSessionDescription d = new mccmsg::TmSessionDescriptionObj((*_currentSession)->name(), "", "", false, {}, {}, bmcl::SystemClock::now(), bmcl::SystemClock::now());
        mccmsg::tmSession::Updater updater(std::move(d), { mccmsg::Field::FinalTime });
        _exchangeService->requestXX(new mccmsg::tmSession::Update_Request(std::move(updater))).then(
            [this](const mccmsg::tmSession::Register_ResponsePtr& rep)
            {
                _currentSession = bmcl::None;
                setStartedIcon(false);
            },
            [this](const mccmsg::ErrorDscr& err)
            {
                BMCL_WARNING() << "Ошибка создания новой сессии: " << err.full();
            }
        );
    }
}

void SessionsWidget::removeSelectedSession()
{
    auto selectedIndexes = _view->selectionModel()->selectedIndexes();
    if (selectedIndexes.empty())
    {
        assert(false);
        return;
    }

    auto sessionVariant = _model->data(selectedIndexes.first(), SessionsModel::SessionId);
    if (sessionVariant.isNull())
    {
        assert(false);
        return;
    }

    auto session = sessionVariant.value<mccmsg::TmSession>();
    _exchangeService->requestXX(new mccmsg::tmSession::UnRegister_Request(session)).then
    (
        [this, session](const mccmsg::tmSession::UnRegister_ResponsePtr& rep)
        {
            onTmSessionRegistered(session, false);
        },
        [this](const mccmsg::ErrorDscr& err)
        {
            BMCL_WARNING() << "Ошибка удаления сессии: " << err.full();
        }
    );
}

void SessionsWidget::currentRowChanged(const QModelIndex &current, const QModelIndex &previous)
{
    _removeSessionAction->setEnabled(current.isValid());
}

void SessionsWidget::setStartedIcon(bool isStarted)
{
    QString btnIcon = isStarted ? ":/toolbar-sessions/stop.png" : ":/toolbar-sessions/start.png";
    _playButton->setIcon(QIcon(btnIcon));
    _recordLabel->setVisible(isStarted);
    _sessionName->setVisible(isStarted);
    _sessionTime->setVisible(isStarted);
    _sessionTool->setSessionDescription(_currentSession);
    //_sessionTool->setEnabled(isStarted);
    adjustSize();
}

void SessionsWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
}

void SessionsWidget::timerEvent(QTimerEvent *event)
{
    if(_currentSession.isSome())
    {
        _time = _time.addSecs(1);
        _sessionTime->setText(_time.toString("HH:mm:ss"));
        _sessionTime->setVisible(true);
        _sessionName->setVisible(true);
    }
    else
    {
        _time = QTime(0, 0, 0);
        _sessionTime->setVisible(false);
        _sessionName->setVisible(false);
    }
    adjustSize();
}
