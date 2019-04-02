#include "mcc/plugin/PluginCache.h"

#include "mcc/ui/MainMenuPlugin.h"
#include "mcc/ui/WidgetPlugin.h"

#include "mcc/ui/WidgetUtils.h"
#include "mcc/ui/Settings.h"
#include "mcc/ui/UserNotifier.h"
#include "mcc/uav/UavController.h"
#include "mcc/uav/UavUiController.h"
#include "mcc/uav/GroupsController.h"
#include "mcc/uav/GlobalActions.h"

#include "mcc/qml/QmlToolWindow.h"
#include "mcc/qml/DeviceUiTool.h"
#include "OpenQmlFileDialog.h"

#include "mcc/msg/obj/Device.h"

#include <bmcl/Utils.h>
#include <bmcl/Logging.h>
#include <bmcl/OptionUtils.h>

#include <QObject>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableView>

struct QmlEntity {
    mccmsg::Device device;
    QString        path;

    QmlEntity();
    QmlEntity(bmcl::Option<mccmsg::Device> uav, const QString& path);
    bool operator==(const QmlEntity& other);
};

QmlEntity::QmlEntity() {}
QmlEntity::QmlEntity(bmcl::Option<mccmsg::Device> uav, const QString& path)
    : path(path)
{
    if (uav.isSome())
        device = uav.unwrap();
}

bool QmlEntity::operator==(const QmlEntity& other)
{
    return other.device == device && other.path == path;
}

QDataStream& operator<<(QDataStream& out, const QmlEntity& obj)
{
    out << obj.device.toQString();
    out << obj.path;
    return out;
}

QDataStream& operator>>(QDataStream& in, QmlEntity& obj)
{
    QString deviceStr;
    in >> deviceStr >> obj.path;
    auto res = mccmsg::Device::createFromString(deviceStr);
    if (res.isOk())
    {
        obj.device = (mccmsg::Device)res.take();
    }
    else
    {
        assert(false);
    }
    return in;
}

typedef QList<QmlEntity> QmlSettingsList;

Q_DECLARE_METATYPE(QmlEntity);

class QmlMenuPlugin : public mccui::MainMenuPlugin
{
public:
    QmlMenuPlugin()
        : _windowCount(1)
    {
        QMenu* menu = new QMenu();

        _toolsMenu = menu->addMenu("Польз. инструменты");
        _toolsMenu->setIcon(QIcon(":/qml.png"));
        auto actionLoadCustomTool = _toolsMenu->addAction(QIcon(":/load2.png"), "Загрузить");
        //auto showManagerTool = _toolsMenu->addAction("Менеджер окон...");
        _toolsMenu->addSeparator();
        QObject::connect(actionLoadCustomTool, &QAction::triggered, [this]() { loadCustomTool(_toolsMenu); });
        //QObject::connect(showManagerTool, &QAction::triggered, [this]() { showWindowsManagerDialog(); });

        setMenu(menu);
    }

    void loadCustomTool(QMenu* menu)
    {
        using mccqml::QmlToolWindow;
        if (bmcl::anyNull(_settings, _groupsController, _uavController))
            return;

        QString startPath = _qmlPathWriter->read().toString();
        if (_uavController->uavsCount() == 0)
        {
            QMessageBox::warning(mccui::findMainWindow(), "Открытие пользовательского интерфейса", "Нет ни одного аппарата для привязки пользовательского интерфейса");
            return;
        }
        OpenQmlFileDialog dialog(_uavController, startPath, mccui::findMainWindow());
        if (dialog.exec() == QDialog::Accepted)
        {
            openQmlFile(dialog.path(), dialog.uav());
        }
     }

    void showWindowsManagerDialog()
    {
        QDialog dialog;
        dialog.setWindowTitle("Менеджер пользовательских окон");
        QHBoxLayout* layout = new QHBoxLayout();
        QTableView* windowsView = new QTableView();
        layout->addWidget(windowsView);
        QPushButton* activateButton = new QPushButton("Активировать");
        QPushButton* deleteButton = new QPushButton("Удалить");
        QPushButton* closeButton = new QPushButton("Закрыть");
        QVBoxLayout* buttonsLayout = new QVBoxLayout();
        layout->addLayout(buttonsLayout);
        buttonsLayout->addWidget(activateButton);
        buttonsLayout->addWidget(deleteButton);
        buttonsLayout->addStretch();
        buttonsLayout->addWidget(closeButton);
        
        QObject::connect(closeButton, &QPushButton::pressed, &dialog, &QDialog::accept);

        dialog.setLayout(layout);
        dialog.exec();
    }
private:
    void openQmlFile(const QString& path, mccmsg::Device device)
    {
        auto userTool = new mccqml::QmlToolWindow(_windowCount, _userNotifier.get(), _groupsController.get(), _uavController.get(), _uiController.get(), device, mccui::findMainWindow());
        _windowCount++;
        if (!userTool->load(QUrl::fromLocalFile(path)))
        {
            BMCL_WARNING() << "Error while loading QML";
            QMessageBox::critical(mccui::findMainWindow(), "Ошибка при загрузке QML", QString("Ошибка при загрузке: %1. \n%2").arg(path).arg(userTool->errorString()));

            removeQmlSettings(path);
            delete userTool;
            return;
        }
        addQmlSettings(device, path);

        QObject::connect(userTool, &mccqml::QmlToolWindow::killMe,
                         [this, userTool]()
        {
            removeQmlSettings(userTool->device(), userTool->path());
            _globalActions->removeMainWidget(userTool);
        }
        );
        _globalActions->insertMainWidget(userTool, _toolsMenu);

    }

    void openQmlFile(const QString& path, bmcl::OptionPtr<mccuav::Uav> uav = bmcl::None)
    {
        auto userTool = new mccqml::QmlToolWindow(_windowCount, _userNotifier.get(), _groupsController.get(), _uavController.get(), _uiController.get(), uav, mccui::findMainWindow());
        _windowCount++;
        if (!userTool->load(QUrl::fromLocalFile(path)))
        {
            BMCL_WARNING() << "Error while loading QML";
            QMessageBox::critical(mccui::findMainWindow(), "Ошибка при загрузке QML", QString("Ошибка при загрузке: %1. \n%2").arg(path).arg(userTool->errorString()));

            removeQmlSettings(path);
            delete userTool;
            return;
        }
        addQmlSettings(uav->device(), path);

        QObject::connect(userTool, &mccqml::QmlToolWindow::killMe,
                [this, userTool]()
                {
                    removeQmlSettings(userTool->device(), userTool->path());
                    _globalActions->removeMainWidget(userTool);
                }
        );
        _globalActions->insertMainWidget(userTool, _toolsMenu);
    }

    QmlSettingsList readAllQmlSettings()
    {
        return _settings->read("qml/qmlSettings").value<QmlSettingsList>();
    }

    void writeAllQmlSettings(const QmlSettingsList& lst)
    {
        _settings->tryWrite("qml/qmlSettings", QVariant::fromValue(lst));
    }

    void addQmlSettings(bmcl::Option<mccmsg::Device> uav, const QString& path)
    {
        auto lst = readAllQmlSettings();
        QmlEntity entity(uav, path);
        auto it = std::find(lst.begin(), lst.end(), entity);
        if (it != lst.end())
            return;
        lst.append(entity);
        writeAllQmlSettings(lst);
    }

    void removeQmlSettings(const QString& path)
    {
        QmlSettingsList newSettings;
        for (const auto& entity : readAllQmlSettings())
        {
            if(entity.path == path)
                continue;
            newSettings.append(entity);
        }
        writeAllQmlSettings(newSettings);
    }


    void removeQmlSettings(bmcl::Option<mccmsg::Device> uav, const QString& path)
    {
        QmlSettingsList newSettings = readAllQmlSettings();
        QmlEntity entity(uav, path);
        newSettings.removeAll(entity);
        writeAllQmlSettings(newSettings);
    }
protected:
    bool init(mccplugin::PluginCache* cache) override
    {
        auto settingsData = cache->findPluginData<mccui::SettingsPluginData>();
        auto groupControllerData = cache->findPluginData<mccuav::GroupsControllerPluginData>();
        auto globalActionsData = cache->findPluginData<mccuav::GlobalActionsPluginData>();
        auto uavControllerData = cache->findPluginData<mccuav::UavControllerPluginData>();
        auto uiControllerData = cache->findPluginData<mccuav::UavUiControllerPluginData>();
        auto userNotifierData = cache->findPluginData<mccui::UserNotifierPluginData>();

        if (bmcl::anyNone(settingsData, groupControllerData, globalActionsData, uavControllerData, userNotifierData))
            return false;

        _settings = settingsData->settings();
        _qmlPathWriter = _settings->acquireUniqueWriter("ide/currentQmlPath", qApp->applicationDirPath() + "/qml").unwrap();
        _groupsController = groupControllerData->groupsController();
        _globalActions = globalActionsData->globalActions();
        _uavController = uavControllerData->uavController();
        _uiController = uiControllerData->uavUiController();
        _userNotifier = userNotifierData->userNotifier();
        return true;
    }

    void guiPostInit() override
    {
        for (const auto& cfg : readAllQmlSettings())
        {
            openQmlFile(cfg.path, cfg.device);
        }
    }

private:
    QMenu* _toolsMenu;
    mccui::Rc<mccui::Settings>          _settings;
    mccui::Rc<mccui::SettingsWriter>    _qmlPathWriter;
    mccui::Rc<mccui::UserNotifier>      _userNotifier;
    mccui::Rc<mccuav::GroupsController> _groupsController;
    mccui::Rc<mccuav::UavController>    _uavController;
    mccui::Rc<mccuav::UavUiController>  _uiController;
    mccui::Rc<mccuav::GlobalActions>    _globalActions;
    unsigned _windowCount;
};

class DeviceUiExtensionPlugin : public mccui::DockWidgetPlugin
{
protected:
    virtual bool init(mccplugin::PluginCache* cache) override
    {
        qRegisterMetaType<QmlEntity>("QmlEntity");
        qRegisterMetaType<QmlSettingsList>("QmlSettingsList");
        qRegisterMetaTypeStreamOperators<QmlEntity>("QmlEntity");
        qRegisterMetaTypeStreamOperators<QmlSettingsList>("QmlSettingsList");

        auto uavControllerData = cache->findPluginData<mccuav::UavControllerPluginData>();
        auto groupControllerData = cache->findPluginData<mccuav::GroupsControllerPluginData>();
        auto uavUiControllerData = cache->findPluginData<mccuav::UavUiControllerPluginData>();
        auto userNotifierData = cache->findPluginData<mccui::UserNotifierPluginData>();
        if (bmcl::anyNone(uavControllerData, groupControllerData, uavUiControllerData, userNotifierData))
            return false;

       _uavController = uavControllerData->uavController();
       _groupsController = groupControllerData->groupsController();
       _uavUiController = uavUiControllerData->uavUiController();
       _userNotifier = userNotifierData->userNotifier();
        setWidget(new mccqml::DeviceUiTool(_userNotifier.get(), _uavController.get(), _uavUiController.get(), _groupsController.get()));
        return true;
    }
private:
    mccui::Rc<mccuav::GroupsController> _groupsController;
    mccui::Rc<mccuav::UavController> _uavController;
    mccui::Rc<mccuav::UavUiController> _uavUiController;
    mccui::Rc<mccui::UserNotifier> _userNotifier;
};



void create(mccplugin::PluginCacheWriter* cache)
{
    auto p = std::make_shared<QmlMenuPlugin>();
    cache->addPlugin(p);

    auto p1 = std::make_shared<DeviceUiExtensionPlugin>();
    cache->addPlugin(p1);
}

MCC_INIT_PLUGIN(create)
