#include "mcc/ui/WidgetPlugin.h"

#include <QWidget>
#include <QDialog>

namespace mccui {

WidgetPlugin::WidgetPlugin(QWidget* widget)
    : UiPlugin(WidgetPlugin::id)
    , _widget(widget)
    , _showCallback([](QWidget*) {})
    , _hideCallback([](QWidget*) {})
{
    if (_widget.isSome()) {
        connectDestroyed();
    }
}

WidgetPlugin::WidgetPlugin()
    : WidgetPlugin(nullptr)
{
}

WidgetPlugin::~WidgetPlugin()
{
    if (_widget.isSome()) {
        if (!_widget->parent()) {
            delete _widget.unwrap();
        }
    }
}

void WidgetPlugin::connectDestroyed()
{
    _destroyConn = QObject::connect(_widget.unwrap(), &QWidget::destroyed, [this]() {
        _widget.clear();
    });
}

void WidgetPlugin::setShowRequestCallback(const ShowCallback& callback)
{
    _showCallback = callback;
}

const WidgetPlugin::ShowCallback& WidgetPlugin::showRequestCallback() const
{
    return _showCallback;
}

void WidgetPlugin::setHideRequestCallback(const WidgetPlugin::ShowCallback& callback)
{
    _hideCallback = callback;
}

const WidgetPlugin::ShowCallback&WidgetPlugin::hideRequestCallback() const
{
    return _hideCallback;
}

void WidgetPlugin::setWidget(QWidget* widget)
{
    if (_widget.isSome()) {
        QObject::disconnect(_destroyConn);
    }
    _widget = widget;
    connectDestroyed();
}

bool WidgetPlugin::hasWidget() const
{
    return _widget.isSome();
}

bmcl::OptionPtr<QWidget> WidgetPlugin::widget()
{
    return _widget;
}

WidgetPlugin::Location DockWidgetPlugin::location() const
{
    return WidgetPlugin::MainWidget;
}

WidgetPlugin::Location ToolBarPlugin::location() const
{
    return WidgetPlugin::ToolBarWidget;
}

}
