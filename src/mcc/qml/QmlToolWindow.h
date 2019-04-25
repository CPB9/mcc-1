#pragma once

#include "mcc/Config.h"
#include "mcc/ui/Fwd.h"
#include "mcc/uav/Fwd.h"
#include "mcc/msg/Objects.h"
#include "mcc/ui/Settings.h"

#include <bmcl/Fwd.h>

#include <QWidget>

class QQuickItem;
class QQuickView;
class QFileSystemWatcher;
class QUrl;

namespace mccqml {

class DeviceUiWidget;

class MCC_QML_DECLSPEC QmlToolWindow : public QWidget
{
    Q_OBJECT

public:
    QmlToolWindow(unsigned windowCount,
                  mccui::Settings* settings,
                  mccui::UserNotifier* userNotifier,
                  mccuav::GroupsController* groupsController,
                  mccuav::UavController* uavController,
                  mccuav::UavUiController* uiController,
                  bmcl::OptionPtr<mccuav::Uav> uav,
                  QWidget* parent = 0);

    QmlToolWindow(unsigned windowCount,
                  mccui::Settings* settings,
                  mccui::UserNotifier* userNotifier,
                  mccuav::GroupsController* groupsController,
                  mccuav::UavController* uavController,
                  mccuav::UavUiController* uiController,
                  mccmsg::Device deviceId,
                  QWidget* parent = 0);

    ~QmlToolWindow();

    bmcl::Option<mccmsg::Device> device() const;

    bool load(const QUrl& url);
    QString errorString() const;
    QString path() const;
signals:
    void killMe();

private slots:
    bool reload();

private:
    void init(unsigned windowCount, 
              mccui::Settings* settings,
              mccui::UserNotifier* userNotifier,
              mccuav::GroupsController* groupsController,
              mccuav::UavController* uavController,
              mccuav::UavUiController* uiController);
    void setupUi();

private:

    QWidget*                _updateRequestWidget;
    QFileSystemWatcher*     _fileWatcher;
    DeviceUiWidget*         _uiExtension;
};
}
