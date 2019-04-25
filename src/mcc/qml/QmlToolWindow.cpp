#include "mcc/qml/QmlToolWindow.h"

#include "mcc/qml/QmlWrapper.h"

#include "mcc/qml/QmlDataConverter.h"
#include "mcc/qml/DeviceUiWidget.h"

#include <QDebug>
#include <QQuickView>
#include <QQmlContext>
#include <QQmlEngine>
#include <QFileSystemWatcher>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

namespace mccqml {

QmlToolWindow::QmlToolWindow(unsigned windowCount,
                             mccui::Settings* settings,
                             mccui::UserNotifier* userNotifier,
                             mccuav::GroupsController* groupsController,
                             mccuav::UavController* uavController,
                             mccuav::UavUiController* uiController,
                             bmcl::OptionPtr<mccuav::Uav> uav,
                             QWidget* parent /*= 0*/)
    : QWidget(parent)
{
    init(windowCount, settings, userNotifier, groupsController, uavController, uiController);
    _uiExtension->setDevice(uav->device());
}

QmlToolWindow::QmlToolWindow(unsigned windowCount,
                             mccui::Settings* settings,
                             mccui::UserNotifier* userNotifier,
                             mccuav::GroupsController* groupsController,
                             mccuav::UavController* uavController,
                             mccuav::UavUiController* uiController,
                             mccmsg::Device device,
                             QWidget* parent /*= 0*/)
{
    init(windowCount, settings, userNotifier, groupsController, uavController, uiController);
    _uiExtension->setDevice(device);
}

void QmlToolWindow::init(unsigned windowCount,
                         mccui::Settings* settings,
                         mccui::UserNotifier* userNotifier,
                         mccuav::GroupsController* groupsController,
                         mccuav::UavController* uavController,
                         mccuav::UavUiController* uiController)
{
    using mccuav::UavController;
    setWindowTitle(QString("Окно %1").arg(windowCount++));
    setObjectName(windowTitle());
    _uiExtension = new DeviceUiWidget(settings, userNotifier, groupsController, uavController, uiController, this);
    setupUi();
    _fileWatcher = new QFileSystemWatcher(this);
    connect(_fileWatcher, &QFileSystemWatcher::fileChanged, this,
            [this]()
            {
                _updateRequestWidget->setVisible(true);
            }
    );
    connect(_uiExtension, &DeviceUiWidget::killMe, this, &QmlToolWindow::killMe);
}



QmlToolWindow::~QmlToolWindow()
{
}

bmcl::Option<mccmsg::Device> QmlToolWindow::device() const
{
    return _uiExtension->device();
}

bool QmlToolWindow::load(const QUrl& url)
{
    if (!_uiExtension->load(url))
        return false;

    _fileWatcher->addPath(url.toLocalFile());

    _updateRequestWidget->setVisible(false);

    QString extensionTitle = _uiExtension->windowTitle();
    if (!extensionTitle.isEmpty())
    {
        setWindowTitle(extensionTitle);
        setObjectName(windowTitle());
    }
    resize(_uiExtension->size());
    return true;
}

QString QmlToolWindow::errorString() const
{
    return _uiExtension->errorString();
}

QString QmlToolWindow::path() const
{
    return _uiExtension->url().toLocalFile();
}

void QmlToolWindow::setupUi()
{
    _updateRequestWidget = new QWidget();
    _updateRequestWidget->setObjectName("UpdateRequestWidget");
    _updateRequestWidget->setStyleSheet("QWidget#UpdateRequestWidget { background-color: rgb(255, 255, 170);}\n");

    QHBoxLayout* updateLayout = new QHBoxLayout();
    updateLayout->setContentsMargins(0, 0, 0, 0);

    updateLayout->addWidget(new QLabel("Файл изменен. Перезагрузить?"));
    QPushButton* reloadButton = new QPushButton("Перезагрузить");
    updateLayout->addStretch();
    updateLayout->addWidget(reloadButton);

    _updateRequestWidget->setLayout(updateLayout);
    connect(reloadButton, &QPushButton::pressed, this, &QmlToolWindow::reload);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(_updateRequestWidget);
    layout->addWidget(_uiExtension, 1);
    setLayout(layout);
}

bool QmlToolWindow::reload()
{
    if (!_uiExtension->reload())
        return false;

    _fileWatcher->addPath(_uiExtension->url().toLocalFile());

    _updateRequestWidget->setVisible(false);
    return true;
}
}
