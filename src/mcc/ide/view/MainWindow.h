#pragma once

#include "mcc/Config.h"
#include "mcc/uav/Rc.h"
#include "mcc/uav/Fwd.h"
#include "mcc/ui/Fwd.h"
#include "mcc/plugin/Fwd.h"

#include <QMainWindow>

#include <bmcl/OptionRc.h>

namespace mccide {

class ContainerWidget;
class MainToolBar;

class MCC_IDE_DECLSPEC MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(mccplugin::PluginCache* cache);
    ~MainWindow() override;

    static QString mainWindowName();

private:
    void showEvent(QShowEvent*) override;
    void closeEvent(QCloseEvent*) override;

    void saveAppState();
    void restoreAppState();

    void updateTitle(const mccuav::Uav* uav);

private:
    ContainerWidget* _container;

    bool _settingsLoaded;
    mccide::MainToolBar* _toolBar;

    QString _defaultWindowTitle;

    bmcl::OptionRc<mccui::Settings> _coreSettings;
    bmcl::OptionRc<mccuav::UavController> _uavController;
    bmcl::OptionRc<mccuav::GlobalActions> _globalActions;

    Q_DISABLE_COPY(MainWindow)
};
}
