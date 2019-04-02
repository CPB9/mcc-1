#pragma once

#include "mcc/Config.h"
#include "mcc/uav/Rc.h"
#include "mcc/msg/Objects.h"
#include "mcc/plugin/PluginData.h"

class QAction;
class QObject;
class QWidget;
class QMenu;

namespace mccuav {

class MCC_UAV_DECLSPEC GlobalActions : public RefCountable
{
public:
    GlobalActions(QObject* parent = nullptr);
    ~GlobalActions() override;

    QAction* showAddUavDialogAction() const;
    QAction* showAddChannelDialogAction() const;
    QAction* showChannelEditDialogAction() const;
    QAction* showEditUavDialogAction() const;
    QAction* showRemoveUavDialogAction() const;
    QAction* showCoordinateConverterDialogAction() const;
    QAction* insertMainWidgetAction() const;
    QAction* removeMainWidgetAction() const;

    void showAddUavDialog();
    void showAddChannelDialog();
    void showChannelEditDialog(const mccmsg::Channel& channel);
    void showEditUavDialog(const mccmsg::Device& uav);
    void showRemoveUavDialog(const mccmsg::Device& uav);
    void showCoordinateConverterDialog();
    void insertMainWidget(QWidget* widget, QMenu* menu = nullptr);
    void removeMainWidget(QWidget* widget);

    void setParent(QObject* parent);

private:
    QAction* _showAddUavDialog;
    QAction* _showAddChannelDialog;
    QAction* _showChannelEditDialog;
    QAction* _showEditUavDialog;
    QAction* _showRemoveUavDialog;
    QAction* _showCoordinateConverterDialog;
    QAction* _insertMainWidget;
    QAction* _removeMainWidget;
};

class MCC_UAV_DECLSPEC GlobalActionsPluginData : public mccplugin::PluginData {
public:
    static constexpr const char* id = "mcc::GlobalActionsPluginData";

    GlobalActionsPluginData(GlobalActions* actions);
    ~GlobalActionsPluginData() override;

    GlobalActions* globalActions();
    const GlobalActions* globalActions() const;

private:
    Rc<GlobalActions> _actions;
};


}
