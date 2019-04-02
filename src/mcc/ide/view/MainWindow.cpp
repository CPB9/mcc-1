#include "mcc/ide/view/MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QMenu>
#include <QShowEvent>
#include <QVBoxLayout>

#include "mcc/res/Resource.h"
#include "mcc/plugin/PluginCache.h"
#include "mcc/ide/dialogs/SettingsDialog.h"
#include "mcc/ide/toolbar/MainToolBar.h"
#include "mcc/ide/view/ContainerWidget.h"
#include "mcc/uav/FirmwareWidgetPlugin.h"
#include "mcc/uav/GlobalActions.h"
#include "mcc/uav/UavController.h"
#include "mcc/ui/DialogPlugin.h"
#include "mcc/ui/MainMenuPlugin.h"
#include "mcc/ui/Settings.h"
#include "mcc/ui/SettingsPage.h"
#include "mcc/ui/SettingsPagePlugin.h"
#include "mcc/ui/TextUtils.h"
#include "mcc/ui/UiPlugin.h"
#include "mcc/ui/WidgetPlugin.h"

MCC_INIT_QRESOURCES(ide);

namespace mccide {

using mccui::Settings;

static ads::DockWidgetArea alignmentToDockArea(Qt::Alignment align)
{
    if (align & Qt::AlignLeft) {
        return ads::LeftDockWidgetArea;
    } else if (align & Qt::AlignRight) {
        return ads::RightDockWidgetArea;
    } else if (align & Qt::AlignTop) {
        return ads::TopDockWidgetArea;
    } else if (align & Qt::AlignBottom) {
        return ads::BottomDockWidgetArea;
    }
    return ads::NoDockWidgetArea;
}

MainWindow::MainWindow(mccplugin::PluginCache* cache)
    : _settingsLoaded(false)
    , _toolBar(nullptr)
{
    setObjectName(mainWindowName());

    setStyleSheet(
        "QMenu {"
        "   color: #fafafa;"
        "   background-color: #404040;"
        "   border: 1px solid black;"
        "}"
        "QMenu::item {"
        "    background-color: transparent;"
        "}"
        ""
        "QMenu::item:selected {"
        "    background-color: #000000;"
        "}"
    );

    setWindowIcon(QIcon(":/app_icon.ico"));

#ifdef MCC_BUILD_NUMBER
    _defaultWindowTitle = QString("%1 (версия %2.%3)").arg("МПУ").arg(QString::fromUtf8(MCC_BRANCH)).arg(QString::number(MCC_BUILD_NUMBER));
#else
    _defaultWindowTitle = "МПУ";
#endif
    setWindowTitle(_defaultWindowTitle);

    for (const auto& plugin : cache->plugins()) {
        auto p = dynamic_cast<mccui::UiPlugin*>(plugin.get());
        if (p) {
            p->setMccMainWindow(this);
        }
    }

    QWidget* mainWidget = new QWidget();
    QVBoxLayout* mainWidgetLayout = new QVBoxLayout();
    mainWidget->setLayout(mainWidgetLayout);
    mainWidgetLayout->setMargin(0);
    mainWidgetLayout->setContentsMargins(0, 0, 0, 0);
    mainWidgetLayout->setSpacing(0);
    setCentralWidget(mainWidget);

    _toolBar = new MainToolBar();
    mainWidgetLayout->addWidget(_toolBar);

    _container = new ContainerWidget();
    mainWidgetLayout->addWidget(_container, 1);

    // ToolBar widgets-plugins
    std::vector<mccui::WidgetPlugin*> leftToolbars;
    std::vector<mccui::WidgetPlugin*> rightToolbars;
    for (const auto& plugin : cache->plugins()) {
        if (plugin->hasTypeId(mccui::WidgetPlugin::id)) {
            auto p = static_cast<mccui::WidgetPlugin*>(plugin.get());
            if (p->location() != mccui::WidgetPlugin::ToolBarWidget) {
                continue;
            }
            if (p->alignment() & Qt::AlignRight) {
                rightToolbars.push_back(p);
            } else {
                leftToolbars.push_back(p);
            }
        }
    }

    for (mccui::WidgetPlugin* p : leftToolbars) {
        auto w = p->widget();
        if (w.isSome()) {
            _toolBar->addUserWidget(w.unwrap());
        }
    }
    _toolBar->addStretch();
    for (mccui::WidgetPlugin* p : rightToolbars) {
        auto w = p->widget();
        if (w.isSome()) {
            _toolBar->addUserWidget(w.unwrap(), false);
        }
    }

    _toolBar->mainMenu()->addMenu(_container->widgetsMenu());
    _toolBar->mainMenu()->addMenu(_container->layoutsMenu());

    //auto deviceSettingsAction = settingsMenu->addAction("Настройки трека...");
    //connect(deviceSettingsAction, &QAction::triggered, this, [this]() { _deviceSettings->show();  });
    //connect(_uavController.get(), &mccuav::UavController::showTrackSettingsForDevice, this, [this]() {_deviceSettings->show();  });

    auto uavData = cache->findPluginData<mccuav::UavControllerPluginData>();
    if (uavData.isSome()) {
        _uavController = uavData->uavController();

        connect(uavData->uavController(), &mccuav::UavController::selectionChanged,
                this, &MainWindow::updateTitle);
        connect(uavData->uavController(), &mccuav::UavController::uavDescriptionChanged,
                this, &MainWindow::updateTitle);
    }


    //_deviceSettings = new DeviceSettingsDialog(_uavController.get(), this);

    // Widgets-plugins
    for (const auto& plugin : cache->plugins()) {
        if (plugin->hasTypeId(mccui::WidgetPlugin::id)) {
            auto p = static_cast<mccui::WidgetPlugin*>(plugin.get());
            if (p->location() != mccui::WidgetPlugin::MainWidget) {
                continue;
            }
            p->setShowRequestCallback([this](QWidget* widget) {
                _container->raiseWidget(widget->objectName());
            });
            p->setHideRequestCallback([this](QWidget* widget) {
                _container->reduceWidget(widget->objectName());
            });
            auto w = p->widget();
            if (w.isSome()) {
                _container->addWidget(w.unwrap(), alignmentToDockArea(p->alignment()));
            }
        }
    }

    // Dialog-plugins
    for (const auto& plugin : cache->plugins()) {
        if (plugin->hasTypeId(mccui::DialogPlugin::id)) {
            auto p = static_cast<mccui::DialogPlugin*>(plugin.get());
            auto d = p->dialog();
            if (d != nullptr) {
                _container->addDialog(d);
            }
        }
    }

    connect(_container, &ContainerWidget::requestSave, this, &MainWindow::saveAppState);

    auto actionsData = cache->findPluginData<mccuav::GlobalActionsPluginData>();
    if (actionsData.isSome()) {
        _globalActions = actionsData->globalActions();
        actionsData->globalActions()->setParent(this);
        connect(actionsData->globalActions()->insertMainWidgetAction(), &QAction::triggered, this, [this]() {
                auto widgetVariant = sender()->property("widget");
                if (widgetVariant.isNull() || !widgetVariant.canConvert<QWidget*>())
                    return;

                auto widget = widgetVariant.value<QWidget*>();
                auto menuVariant = sender()->property("menu");
                if (menuVariant.isNull() || !menuVariant.canConvert<QMenu*>())
                {
                    _container->addWidget(widget);
                }
                else
                {
                    auto m = menuVariant.value<QMenu*>();
                    _container->addWidget(widget, ads::CenterDockWidgetArea, false, m);
                }

//                 _container->addFloatingWidget(widget.value<QWidget*>());
//                 _container->raiseWidget(widget.value<QWidget*>()->objectName());
            }
        );

        connect(actionsData->globalActions()->removeMainWidgetAction(), &QAction::triggered, this, [this]() {
            auto widgetVariant = sender()->property("widget");
            if (widgetVariant.isNull() || !widgetVariant.canConvert<QWidget*>())
                return;

            auto widget = widgetVariant.value<QWidget*>();
            _container->removeWidget(widget);
            }
        );
    }

    // Menu-plugins
    for (const auto& plugin : cache->plugins()) {
        if (plugin->hasTypeId(mccui::MainMenuPlugin::id)) {
            auto p = static_cast<mccui::MainMenuPlugin*>(plugin.get());
            auto w = p->takeMenu();
            if (!w)
                continue;
            w->setStyleSheet(styleSheet());
            _toolBar->mainMenu()->addActions(w->actions());
            p->guiPostInit();
            w.release();
        }
    }

    auto settingsData = cache->findPluginData<mccui::SettingsPluginData>();
    if (settingsData.isSome()) {
        _coreSettings.emplace(settingsData->settings());
        auto appSettings = new SettingsDialog(settingsData->settings(), this);

        // Settings pages plugins
        //std::vector<mccui::SettingsPagePlugin*> pagePlugins;
        for (const auto& plugin : cache->plugins()) {
            if (plugin->hasTypeId(mccui::SettingsPagePlugin::id)) {
                auto s = static_cast<mccui::SettingsPagePlugin*>(plugin.get());
                auto page = s->takeSettingsPage();
                page->load();
                appSettings->addPage(page.release());
            }
        }

        auto appSettingsAction = _toolBar->mainMenu()->addAction(mccres::loadIcon(mccres::ResourceKind::SettingsIcon), "Настройки");

        connect(appSettingsAction, &QAction::triggered, this, [appSettings]() {
            appSettings->load();
            appSettings->show();
        });
    }
}

MainWindow::~MainWindow()
{
    delete _container;
}

QString MainWindow::mainWindowName()
{
    return QString("mccide::MainWindow");
}

void MainWindow::showEvent(QShowEvent* e)
{
    QMainWindow::showEvent(e);
    qDebug() << "size: " << size();
    if (_settingsLoaded)
        return;

    restoreAppState();
    qDebug() << "loaded size: " << size();

    _settingsLoaded = true;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    saveAppState();
    QMainWindow::closeEvent(e);
    qApp->closeAllWindows();
}

void MainWindow::updateTitle(const mccuav::Uav* uav)
{
    if(uav == nullptr)
    {
        setWindowTitle(_defaultWindowTitle);
    }
    else
    {
        QString deviceName = uav->getName();
        if(deviceName.isEmpty())
        {
            deviceName = uav->device().toQString().remove("{").left(8);
        }
        else
        {
            deviceName = mccui::shortTextLine(deviceName);
        }

        setWindowTitle(QString("%1 — %2").arg(deviceName).arg(_defaultWindowTitle));
    }
}

static const char* posKey = "window/geometry/pos";
static const char* sizeKey = "window/geometry/size";
static const char* maximizedKey = "window/geometry/maximized";
static const char* windowStateKey = "window/state";
static const char* dockingStateKey = "window/dockingState";

void MainWindow::saveAppState()
{
    if (!_settingsLoaded || _coreSettings.isNone())
        return;

    qDebug() << "Сохранение состояния приложения";

    auto settings = _coreSettings.unwrap().get();
    settings->tryWrite(posKey, pos());
    settings->tryWrite(sizeKey, size());
    settings->tryWrite(maximizedKey, isMaximized());
    settings->tryWrite(windowStateKey, saveState());
    settings->tryWrite(dockingStateKey, _container->saveState());
}

struct WindowGeometry {
    QPoint position;
    QSize size;
    bool maximized;

    WindowGeometry() : size(640, 480), maximized(false)
    {
    }
};


void MainWindow::restoreAppState()
{
    if (_coreSettings.isNone()) {
        return;
    }
    auto settings = _coreSettings.unwrap().get();
    QRectF screenGeometry = QApplication::desktop()->screenGeometry();
    WindowGeometry geometry;

    auto pos = settings->read("window/geometry/pos");
    auto size = settings->read("window/geometry/size");
    auto maximized = settings->read("window/geometry/maximized");

    if (pos.isValid() && size.isValid() && maximized.isValid()) {
        geometry.position = pos.toPoint();
        geometry.size = size.toSize();
        geometry.maximized = maximized.toBool();
    }

    if (!screenGeometry.intersects(QRect(geometry.position, geometry.size))) {
        geometry.position = QPoint(0, 0);
    }

    move(geometry.position);
    resize(geometry.size);
    if (geometry.maximized) {
        setWindowState(windowState() | Qt::WindowMaximized);
    }

    restoreState(settings->read(windowStateKey).toByteArray());
    auto dockingState = settings->read(dockingStateKey).toByteArray();
    if (!dockingState.isEmpty()) {
        _container->restoreState(dockingState);
    }
}
}
