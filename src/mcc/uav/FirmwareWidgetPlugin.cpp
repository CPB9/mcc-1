#include "mcc/uav/FirmwareWidgetPlugin.h"

#include <QWidget>

namespace mccuav {

FirmwareWidgetPlugin::FirmwareWidgetPlugin(const mccmsg::Protocol& protocol)
    : UiPlugin(id)
    , _protocol(protocol)
{

}

void FirmwareWidgetPlugin::setWidget(QWidget* widget)
{
    _widget.reset(widget);
}

const mccmsg::Protocol& FirmwareWidgetPlugin::protocol() const
{
    return _protocol;
}

std::unique_ptr<QWidget> FirmwareWidgetPlugin::takeWidget()
{
    return std::move(_widget);
}
}
