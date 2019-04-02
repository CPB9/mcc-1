#pragma once

#include "mcc/Config.h"
#include "mcc/ui/UiPlugin.h"

#include <bmcl/OptionPtr.h>

#include <QMetaObject>

#include <functional>

class QWidget;

namespace mccui {

class WidgetPlugin;

typedef std::shared_ptr<WidgetPlugin> WidgetPluginPtr;

class MCC_UI_DECLSPEC WidgetPlugin : public UiPlugin {
public:
    using ShowCallback = std::function<void(QWidget* widget)>;
    using HideCallback = std::function<void(QWidget* widget)>;

    enum Location {
        MainWidget,
        ToolBarWidget,
    };

    static constexpr const char* id = "mcc::WidgetPlugin";

    explicit WidgetPlugin();
    explicit WidgetPlugin(QWidget* widget);
    ~WidgetPlugin() override;

    virtual Location location() const = 0;
    bool hasWidget() const;
    bmcl::OptionPtr<QWidget> widget();
    void setWidget(QWidget* widget);

    void setShowRequestCallback(const ShowCallback& callback);
    const ShowCallback& showRequestCallback() const;

    void setHideRequestCallback(const ShowCallback& callback);
    const ShowCallback& hideRequestCallback() const;

private:
    void connectDestroyed();

    bmcl::OptionPtr<QWidget>    _widget;
    ShowCallback                _showCallback;
    HideCallback                _hideCallback;
    QMetaObject::Connection     _destroyConn;
};

class MCC_UI_DECLSPEC DockWidgetPlugin : public WidgetPlugin {
public:
    using WidgetPlugin::WidgetPlugin;

    WidgetPlugin::Location location() const override;
};

class MCC_UI_DECLSPEC ToolBarPlugin : public WidgetPlugin {
public:
    using WidgetPlugin::WidgetPlugin;

    WidgetPlugin::Location location() const override;
};
}
