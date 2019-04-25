#pragma once

#include "mcc/Config.h"
#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"
#include "mcc/uav/Uav.h" //TODO: отвязaться
#include "mcc/uav/UavUi.h"
#include "mcc/uav/Fwd.h"

#include <QWidget>
#include <QUrl>

class QQuickView;
class QQuickItem;
class QTableView;
class QLabel;
class QToolBar;

namespace mccqml {

class QmlWrapper;
class QmlDataConverter;

class MCC_QML_DECLSPEC DeviceUiWidget : public QWidget
{
    Q_OBJECT

public:
    DeviceUiWidget(mccui::Settings* settings, 
                   mccui::UserNotifier* notifier,
                   mccuav::GroupsController* groupsController,
                   mccuav::UavController* uavController,
                   mccuav::UavUiController* uiController,
                   QWidget* parent);
    ~DeviceUiWidget();

    bool load(const QUrl& url);
    void tryToLoad();
    bool reload(const QUrl& url);
    bool reload();

    QUrl url() const;
    QString rootPath() const;
    QString errorString() const;
    mccmsg::Device device() const;
    void setDevice(mccmsg::Device device);
    void setLocalCopy(bool localCopy);

    void setEnableSwitchLocalOnboard(bool enabled);

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void killMe();
    void showInShellPressed();
    void switchUiToLocal();
    void switchUiToOnboard();

private slots:
    void updateTitle();
    void onLocalHashChanged();
    void showContextMenu(const QPoint& pos);
private:
    mccuav::Rc<mccuav::UavController> _uavController;
    mccuav::Rc<mccuav::UavUiController> _uiController;
    mccuav::Rc<mccui::UserNotifier>     _userNotifier;
    mccui::Rc<mccui::SettingsWriter>    _showToolbarWriter;

    bool            _isLoaded;
    bool            _isCustom;
    QUrl            _url;
    mccmsg::Device  _device;

    QWidget*          _rootWidget;
    QQuickView*       _view;
    QTableView*       _warningsView;
    QmlWrapper*       _commonWrapper;
    QmlDataConverter* _dataConverter;

    QAction*          _killAction;
    QAction*          _showInFolder;
    QAction*          _switchLocalOnboard;
    QLabel*           _currentUavName;
    QAction*          _reloadAction;
    QToolBar*         _toolbar;
};
}
