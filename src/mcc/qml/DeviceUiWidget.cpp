#include "mcc/qml/DeviceUiWidget.h"

#include <QDebug>
#include <QQuickView>
#include <QQmlContext>
#include <QQmlEngine>
#include <QVBoxLayout>
#include <QToolBar>
#include <QTableView>
#include <QSplitter>
#include <QPushButton>
#include <QAction>
#include <QLabel>
#include <QMessageBox>
#include <QQuickItem>
#include <QQmlProperty>

#include <fmt/format.h>

#include "mcc/qml/QmlWrapper.h"
#include "mcc/qml/QmlDataConverter.h"
#include "mcc/qml/VideoImageProvider.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/UavUiController.h"
#include "mcc/uav/UavUi.h"
#include "mcc/msg/TmView.h"
#include "mcc/res/Resource.h"
#include "mcc/ui/WidgetUtils.h"
#include "mcc/ui/UserNotifier.h"

namespace mccqml {

DeviceUiWidget::DeviceUiWidget(mccui::UserNotifier* notifier,
                               mccuav::GroupsController* groupsController,
                               mccuav::UavController* uavController,
                               mccuav::UavUiController* uiController,
                               QWidget* parent)
    : QWidget(parent)
    , _userNotifier(notifier)
    , _uavController(uavController)
    , _uiController(uiController)
    , _isLoaded(false)
    , _isCustom(false)
{
    _dataConverter = new QmlDataConverter;
    _view = new QQuickView();
    _view->engine()->addImportPath(QCoreApplication::applicationDirPath() + "/qml");

    connect(_view->engine(), &QQmlEngine::warnings, this,
            [this](const QList<QQmlError> &warnings)
            {
                for(const auto& e : warnings)
                {
                    _uavController->onLog(bmcl::LogLevel::Warning, fmt::format("{}: {}", e.url().fileName().toStdString(), e.description().toStdString()));
                }
            });

    _rootWidget = QWidget::createWindowContainer(_view);
    _warningsView = new QTableView();

//     auto splitter = new QSplitter();
//     splitter->setOrientation(Qt::Vertical);
//     splitter->addWidget(_rootWidget);
//     splitter->addWidget(_warningsView);
    auto toolbar = new QToolBar();
    toolbar->setIconSize(QSize(32, 32));
    toolbar->setFixedHeight(32);
    _killAction = toolbar->addAction("Выгрузить");
    _killAction->setIcon(mccres::loadIcon(mccres::ResourceKind::CancelButtonIcon));
    _killAction->setToolTip("Закрыть текущее окно");

    connect(_killAction, &QAction::triggered, this, &DeviceUiWidget::killMe);

    _showInFolder = toolbar->addAction("Расположение копии");
    _showInFolder->setIcon(mccres::loadIcon(mccres::ResourceKind::FolderLocation));
    connect(_showInFolder, &QAction::triggered, this, [this]() { mccui::showInGraphicalShell(_view->source().toLocalFile()); });

    _switchLocalOnboard = toolbar->addAction("Локальная копия");
    _switchLocalOnboard->setCheckable(true);
    _switchLocalOnboard->setIcon(mccres::loadIcon(mccres::ResourceKind::OnboardFile));
    connect(_switchLocalOnboard, &QAction::triggered, this,
            [this]()
            {
                if (_isCustom)
                {
                    assert(false);
                    return;
                }
                auto ui = _uiController->ui(_device);
                if (ui.isNone())
                {
                    assert(false);
                    return;
                }
                if (ui->isOnboard())
                {
                    if (!ui->localCopyExists() && QMessageBox::question(mccui::findMainWindow(), "Интерфейс аппарата", "Локальная копия интерфейса не существует.\nСоздать локальную копию интерфейса?") == QMessageBox::Yes)
                    {
                        ui->createLocalCopy();
                    }
                    if (ui->localCopyExists())
                    {
                        ui->switchToLocalCopy();
                        reload(QUrl::fromLocalFile(ui->localPath()));
                        _switchLocalOnboard->setIcon(mccres::loadIcon(mccres::ResourceKind::UserFile));
                        connect(ui.unwrap().get(), &mccuav::UavUi::localHashChanged, this, &DeviceUiWidget::onLocalHashChanged);
                    }
                }
                else
                {
                    disconnect(ui.unwrap().get(), &mccuav::UavUi::localHashChanged, this, &DeviceUiWidget::onLocalHashChanged);
                    ui->switchToOnboard();
                    _switchLocalOnboard->setIcon(mccres::loadIcon(mccres::ResourceKind::OnboardFile));
                    reload(QUrl::fromLocalFile(ui->originPath()));
                }
            });

    _currentUavName = new QLabel(this);
    toolbar->addWidget(_currentUavName);
    _reloadAction = toolbar->addAction("Перезагрузить...");
    connect(_reloadAction, &QAction::triggered, this, [this]() { _reloadAction->setVisible(false); reload(); });
    _reloadAction->setVisible(false);
    _reloadAction->setIcon(mccres::loadIcon(mccres::ResourceKind::ReloadActiveIcon));
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(toolbar);
    layout->addWidget(_rootWidget, 1);
    setLayout(layout);

    _commonWrapper = new QmlWrapper(groupsController, uavController, this);
    connect(_commonWrapper, &QmlWrapper::registerVideoProviderInEngine, this,
        [this](const QString& name, VideoImageProvider* provider)
        {
            _view->engine()->addImageProvider(name, (QQmlImageProviderBase*)provider);
        });

    connect(_commonWrapper, &QmlWrapper::unregisterVideoProviderInEngine, this,
        [this](const QString& name)
    {
        _view->engine()->removeImageProvider(name);
    });

    connect(_uavController.get(), &mccuav::UavController::uavFirmwareLoaded, this,
            [this](mccuav::Uav* device)
            {
                if(_isLoaded)
                    return;

                if (device->device() == _device && !_url.isEmpty())
                {
                    tryToLoad();
                }
            }
    );

    connect(_uavController.get(), &mccuav::UavController::uavRemoved, this,
            [this](mccuav::Uav* device)
            {
                if (_device == device->device())
                    emit killMe();
            }
    );

    setEnableSwitchLocalOnboard(false);
}

DeviceUiWidget::~DeviceUiWidget()
{
    delete _view;
    delete _dataConverter;
}

bool DeviceUiWidget::load(const QUrl& url)
{
    if (!url.isLocalFile())
    {
        Q_ASSERT(false);
        qDebug() << "Возможно открыть только локальный файл: " << url.toString();
        return false;
    }

    _url = url;
    tryToLoad();
    return true;
}

void DeviceUiWidget::tryToLoad()
{
    if (_isLoaded)
        return;

    auto uav = _uavController->uav(_device);
    if (uav.isNone())
        return;

    _view->rootContext()->setContextProperty("mcc",             _commonWrapper);
    _view->rootContext()->setContextProperty("dataConverter",   _dataConverter);
    _view->rootContext()->setContextProperty("__uavController", QVariant::fromValue(_uavController.get()));
    _view->rootContext()->setContextProperty("__deviceId",      QVariant::fromValue(uav->deviceId()));
    _view->rootContext()->setContextProperty("app",             QVariant::fromValue(_userNotifier.get()));
    _view->rootContext()->setContextProperty("rootPath",        rootPath());
    _view->setSource(_url);

    if(_view->status() != QQuickView::Ready)
    {
        if(!_uavController.isNull())
            _uavController->onLog(bmcl::LogLevel::Critical, errorString().toStdString());

        _reloadAction->setVisible(true);
        return;
    }

    QQuickItem* container = _view->rootObject();
    resize(container->width(), container->height());

    updateTitle();
    auto title = container->property("title");
    if (title.isValid())
    {
        setWindowTitle(title.toString());
        setObjectName(title.toString());
    }

    _isLoaded = true;
}

bool DeviceUiWidget::reload(const QUrl& url)
{
    _isLoaded = false;
    _view->setSource(QUrl());
    _view->engine()->clearComponentCache();
    _commonWrapper->reset();

    return load(url);
}

bool DeviceUiWidget::reload()
{
    return reload(url());
}

QUrl DeviceUiWidget::url() const
{
    return _view->source();
}

QString DeviceUiWidget::rootPath() const
{
    QFileInfo info(_url.toLocalFile());
    return info.absoluteDir().path();
}

QString DeviceUiWidget::errorString() const
{
    QString errorString = _view->source().path() + " ";
    auto errors = _view->errors();

    for (auto e : errors)
    {
        qDebug() << e.description();
        errorString += QString("[%1: %2]: %3\n").arg(e.line()).arg(e.column()).arg(e.description());
    }

    return errorString;
}

mccmsg::Device DeviceUiWidget::device() const
{
    return _device;
}

void DeviceUiWidget::setDevice(mccmsg::Device device)
{
    assert(!_isLoaded);
    _device = device;
}

void DeviceUiWidget::setEnableSwitchLocalOnboard(bool enabled)
{
    _killAction->setVisible(!enabled);
    _switchLocalOnboard->setVisible(enabled);
    _isCustom = !enabled;
}

void DeviceUiWidget::updateTitle()
{
    auto uav = _uavController->uav(_device);
    if (uav.isNone())
    {
        return;
    }
    QString uavInfo = uav->getInfo();

    if (_isCustom)
    {
        _currentUavName->setText(QString("%1 [%2, пользовательский]").arg(uavInfo).arg(uav->deviceId()));
        return;
    }
    auto uavUi = _uiController->ui(_device);
    if (uavUi.isNone())
    {
        _currentUavName->setText(uavInfo);
    }
    else
    {
        QString type;
        switch (uavUi->currentType())
        {
            case mccuav::UavUi::Type::Onboard: 
            {
                type = "бортовой";
                break;
            }
            case mccuav::UavUi::Type::LocalCopy:
            {
                type = "локальная копия";
                if(uavUi->hasLocalChanges())
                    type += " - есть изменения!";
                break;
            }
            case mccuav::UavUi::Type::Custom:
            {
                type = "пользовательский";
                break;
            }
        }

        _currentUavName->setText(QString("%1 [%2, %3]").arg(uavInfo).arg(uav->deviceId()).arg(type));
    }
}

void DeviceUiWidget::onLocalHashChanged()
{
    updateTitle();
    _reloadAction->setVisible(true);
}

}
