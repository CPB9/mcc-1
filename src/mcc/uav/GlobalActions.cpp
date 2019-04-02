#include "mcc/uav/GlobalActions.h"

#include <QAction>
#include <QUuid>
#include <QMenu>

namespace mccuav {

GlobalActions::GlobalActions(QObject* parent)
  : _showAddUavDialog(new QAction(parent))
  , _showAddChannelDialog(new QAction(parent))
  , _showChannelEditDialog(new QAction(parent))
  , _showEditUavDialog(new QAction(parent))
  , _showRemoveUavDialog(new QAction(parent))
  , _showCoordinateConverterDialog(new QAction(parent))
  , _insertMainWidget(new QAction(parent))
  , _removeMainWidget(new QAction(parent))
{}

GlobalActions::~GlobalActions()
{}

void GlobalActions::setParent(QObject* parent)
{
    _showAddUavDialog->setParent(parent);
    _showAddChannelDialog->setParent(parent);
    _showChannelEditDialog->setParent(parent);
    _showEditUavDialog->setParent(parent);
    _showRemoveUavDialog->setParent(parent);
    _insertMainWidget->setParent(parent);
}

QAction* GlobalActions::showAddUavDialogAction() const
{
    return _showAddUavDialog;
}

QAction*GlobalActions::showAddChannelDialogAction() const
{
    return _showAddChannelDialog;
}

QAction* GlobalActions::showChannelEditDialogAction() const
{
    return _showChannelEditDialog;
}

QAction*GlobalActions::showEditUavDialogAction() const
{
    return _showEditUavDialog;
}

QAction*GlobalActions::showRemoveUavDialogAction() const
{
    return _showRemoveUavDialog;
}

QAction*GlobalActions::showCoordinateConverterDialogAction() const
{
    return _showCoordinateConverterDialog;
}

QAction* GlobalActions::insertMainWidgetAction() const
{
    return _insertMainWidget;
}

QAction* GlobalActions::removeMainWidgetAction() const
{
    return _removeMainWidget;
}

void GlobalActions::insertMainWidget(QWidget* widget, QMenu* menu)
{
    _insertMainWidget->setProperty("widget", QVariant::fromValue(widget));
    _insertMainWidget->setProperty("menu", QVariant::fromValue(menu));
    _insertMainWidget->trigger();
}

void GlobalActions::removeMainWidget(QWidget* widget)
{
    _removeMainWidget->setProperty("widget", QVariant::fromValue(widget));
    _removeMainWidget->trigger();
}

void GlobalActions::showChannelEditDialog(const mccmsg::Channel& channel)
{
    _showChannelEditDialog->setData(channel.toQUuid());
    _showChannelEditDialog->trigger();
}

void GlobalActions::showEditUavDialog(const mccmsg::Device& uav)
{
    _showEditUavDialog->setData(uav.toQUuid());
    _showEditUavDialog->trigger();
}

void GlobalActions::showRemoveUavDialog(const mccmsg::Device& uav)
{
    _showRemoveUavDialog->setData(uav.toQUuid());
    _showRemoveUavDialog->trigger();
}

void GlobalActions::showCoordinateConverterDialog()
{
    _showCoordinateConverterDialog->trigger();
}

void GlobalActions::showAddUavDialog()
{
    _showAddUavDialog->trigger();
}

void GlobalActions::showAddChannelDialog()
{
    _showAddChannelDialog->trigger();
}

GlobalActionsPluginData::GlobalActionsPluginData(GlobalActions* actions)
    : mccplugin::PluginData(id)
    , _actions(actions)
{}

GlobalActionsPluginData::~GlobalActionsPluginData()
{}

GlobalActions* GlobalActionsPluginData::globalActions()
{
    return _actions.get();
}

const GlobalActions* GlobalActionsPluginData::globalActions() const
{
    return _actions.get();
}
}
