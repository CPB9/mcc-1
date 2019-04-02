#pragma once

#include "mcc/Config.h"
#include "mcc/ui/UiPlugin.h"
#include "mcc/msg/Objects.h"

class QWidget;

namespace mccuav {

class FirmwareWidgetPlugin;

typedef std::shared_ptr<FirmwareWidgetPlugin> FirmwareWidgetPluginPtr;

class MCC_UAV_DECLSPEC FirmwareWidgetPlugin : public mccui::UiPlugin {
public:
    static constexpr const char* id = "mcc::FirmwareWidgetPlugin";

    FirmwareWidgetPlugin(const mccmsg::Protocol& protocol);

    void setWidget(QWidget* widget);

    const mccmsg::Protocol& protocol() const;
    std::unique_ptr<QWidget> takeWidget();

private:
    mccmsg::Protocol _protocol;
    std::unique_ptr<QWidget> _widget;
};

}
