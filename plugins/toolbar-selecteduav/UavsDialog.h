#pragma once

#include <cstddef>
#include "mcc/msg/Objects.h"
#include "mcc/uav/Fwd.h"
#include "mcc/ui/Dialog.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Rc.h"

#include <QFrame>

class QLabel;
class QMenu;
class QScrollArea;

class ToolbarUavSettingsPage;
class UavWidget;

namespace mccide {
class AddEntityWidget;
}

namespace mccui{
class ClickableLabel;
}

class UavsDialog : public mccui::Dialog
{
    Q_OBJECT
public:
    explicit UavsDialog(mccuav::UavController* uavController,
                        mccuav::GlobalActions* actions,
                        ToolbarUavSettingsPage* settings,
                        QWidget *parent = nullptr);
    ~UavsDialog() override;

    bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void updateDeviceDescription(const mccuav::Uav* vehicle);
    void updateList();
    void setAddVehicleVisible(bool visible);
    bool isAddVehicleVisible() const;
    void execVehicleMenu(const mccmsg::Device device);

protected:
    void showEvent(QShowEvent *event) override;

private:
    mccui::Rc<mccuav::UavController>    _uavController;
    mccui::Rc<mccuav::GlobalActions>    _actions;
    ToolbarUavSettingsPage*             _settings;

    std::vector<UavWidget*>             _uavWidgets;
    mccide::AddEntityWidget*            _addUavWidget;

    QScrollArea*                        _view;

    QMenu*                              _uavMenu;
    mccmsg::Device                      _menuDevice;

    Q_DISABLE_COPY(UavsDialog)
};
