#pragma once

#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"
#include "mcc/uav/Fwd.h"

#include <QWidget>

#include "mcc/msg/Objects.h"

class QStackedWidget;
class QListWidget;

namespace mccmav {

class FirmwareWidget : public QWidget
{
public:
    FirmwareWidget(const mccmsg::Protocol& protocol,
                   mccuav::ChannelsController* chanController,
                   mccuav::UavController* uavController);
    ~FirmwareWidget();

private:
    void addPage(const QString& section, QWidget* contents);
private:
    mccmsg::Protocol _protocol;

    QListWidget* _sectionsWidget;
    QStackedWidget* _contentsWidget;
    mccui::Rc<mccuav::UavController> _uavController;
};
}
