#include "SessionsWidget.h"
#include "SessionsModel.h"

#include "mcc/msg/obj/TmSession.h"
#include "mcc/msg/ptr/TmSession.h"

#include <bmcl/MakeRc.h>
#include <QVBoxLayout>
#include <QTableView>
#include <QToolBar>
#include <QAction>

SessionsWidget::SessionsWidget(const mccui::Rc<mccuav::ExchangeService>& exchangeService,
                               const mccui::Rc<mccuav::UavController>& uavController,
                               const mccui::Rc<mccuav::ChannelsController>& channelsController)
    : _exchangeService(exchangeService)
    , _uavController(uavController)
    , _channelsController(channelsController)
{
    setWindowTitle("Управление сессиями");
    setObjectName(windowTitle());
    setWindowIcon(QIcon(":/toolbar-sessions/icon.png"));

    _toolbar = new QToolBar();
    auto startSessionAction = _toolbar->addAction("Начать новую сессию");
    _removeSessionAction = _toolbar->addAction("Удалить сессию");
    _removeSessionAction->setEnabled(false);

    connect(startSessionAction, &QAction::triggered, this, &SessionsWidget::startNewSession);
    connect(_removeSessionAction, &QAction::triggered, this, &SessionsWidget::removeSelectedSession);

    auto layout = new QVBoxLayout();
    setLayout(layout);

    layout->addWidget(_toolbar);
    _model = new SessionsModel(_uavController, channelsController);

    _view = new QTableView();
    _view->setModel(_model);

    layout->addWidget(_view);

    connect(_view->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &SessionsWidget::currentRowChanged);

    exchangeService->requestXX(new mccmsg::tmSession::DescriptionList_Request).then
    (
        [this](const mccmsg::tmSession::DescriptionList_ResponsePtr& rep)
        {
            for (const auto& r : rep->data())
            {
                _model->updateSession(r);
            }
        },
        [this](const mccmsg::ErrorDscr& err)
        {
            BMCL_WARNING() << "Ошибка запроса списка сессий: " << err.full();
            //QMessageBox::warning(this, "Ошибка при создании канала обмена", "channelRegisterFailed: " + err.qfull());
        }
    );

    connect(_exchangeService.get(), &mccuav::ExchangeService::tmSessionRegistered, this, &SessionsWidget::onTmSessionRegistered);
    connect(_exchangeService.get(), &mccuav::ExchangeService::tmSessionUpdated, this, &SessionsWidget::onTmSessionUpdated);
}

void SessionsWidget::onTmSessionUpdated(const mccmsg::TmSessionDescription& session)
{
    _model->updateSession(session);
}

void SessionsWidget::onTmSessionRegistered(const mccmsg::TmSession& session, bool isRegistered)
{
    if(!isRegistered)
    {
        _model->removeSession(session);
        return;
    }

    _exchangeService->requestXX(new mccmsg::tmSession::Description_Request(session)).then
    (
        [this](const mccmsg::tmSession::Description_ResponsePtr& rep)
        {
            _model->updateSession(rep->data());
        },
        [this, &session](const mccmsg::ErrorDscr& err)
        {
            BMCL_WARNING() << "Ошибка запроса описания сессии " << session.toStdString() << ": " << err.full();
        }
    );

}

void SessionsWidget::startNewSession()
{
    auto dscr = bmcl::makeRc<const mccmsg::TmSessionDescriptionObj>(mccmsg::TmSession::createNil(), "тестовое пустое описание сессии");
    _exchangeService->requestXX(new mccmsg::tmSession::Register_Request(std::move(dscr))).then
    (
        [this](const mccmsg::tmSession::Register_ResponsePtr& rep)
        {
            _model->updateSession(rep->data());
        },
        [this](const mccmsg::ErrorDscr& err)
        {
            BMCL_WARNING() << "Ошибка создания новой сессии: " << err.full();
        }
    );
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
