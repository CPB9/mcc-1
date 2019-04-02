#include "mcc/ide/view/PrimaryFlightDisplayTool.h"
#include "mcc/ui/PrimaryFlightDisplay.h"

#include <QVBoxLayout>

namespace mccide {

PrimaryFlightDisplayTool::PrimaryFlightDisplayTool(mcc::ui::mccuav::UavController* uavController, QWidget* parent)
    : ToolWindow(uavController, parent)
{
    using mcc::ui::mccui::PrimaryFlightDisplay;

    setObjectName("Primary Flight Display");
    setWindowTitle("Командно-пилотажный прибор");

    _pfd = new PrimaryFlightDisplay(uavController);
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_pfd);
    setLayout(layout);
}

mcc::ui::mccui::PrimaryFlightDisplay* PrimaryFlightDisplayTool::pfd()
{
    return _pfd;
}

void PrimaryFlightDisplayTool::currentDeviceChangedPrivate(mcc::ui::mccui::FlyingDevice* device)
{
    _pfd->setModel(device);
}

}
