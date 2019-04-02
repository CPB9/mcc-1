#pragma once

#include "mcc/Config.h"
#include "mcc/ui/Fwd.h"
#include "mcc/uav/Fwd.h"
#include "mcc/msg/Objects.h"
#include <bmcl/OptionPtr.h>

#include <QWidget>
#include <QMap>

class QStackedLayout;
class QTemporaryDir;

namespace mccqml {

class MCC_QML_DECLSPEC DeviceUiTool : public QWidget
{
    Q_OBJECT

public:
    DeviceUiTool(mccui::UserNotifier* userNotifier, mccuav::UavController* uavController, mccuav::UavUiController* uiController, mccuav::GroupsController* groupsController, QWidget* parent = nullptr);
    ~DeviceUiTool();

private slots:
    void deviceUiUpdated(mccmsg::Device device, const bmcl::OptionRc<mccuav::UavUi>& ui);
    void onUiTypeChanged(mccmsg::Device device);
    void onDeviceRemoved(mccuav::Uav* uav);
private:
    void onDeviceChanged(mccuav::Uav* device);

private:
    QTemporaryDir * _uiDir;
    QStackedLayout* _layout;

    mccuav::UavController* _uavController;
    mccuav::UavUiController* _uiController;
    mccuav::GroupsController* _groupsController;
    mccui::UserNotifier* _userNotifier;
    QMap<mccuav::Uav*, int> _indexMap;
};
}
