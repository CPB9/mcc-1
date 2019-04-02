#pragma once

#include "mcc/ui/Rc.h"
#include "mcc/ui/Fwd.h"
#include "mcc/uav/Fwd.h"

#include <QWidget>

class QComboBox;
class QLabel;

class ToolbarUavSettingsPage;
class UavWidget;
class UavsDialog;

class SelectedUavWidget : public QWidget
{
    Q_OBJECT
public:
    SelectedUavWidget(mccuav::GroupsController* groupsController,
                      mccuav::UavController* uavController,
                      mccuav::GlobalActions* actions,
                      ToolbarUavSettingsPage* settings,
                      QWidget* parent = nullptr);
    ~SelectedUavWidget() override;

    bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
    void updateDeviceDescription(const mccuav::Uav* vehicle);
    void updateSelection();

private:
    void updateMinimumWidth();

    mccui::Rc<mccuav::GroupsController> _groupsController;
    mccui::Rc<mccuav::UavController>    _uavController;
    mccui::Rc<mccuav::GlobalActions>    _actions;
    ToolbarUavSettingsPage*             _settings;

    mccui::ClickableLabel*              _noUavsLabel;
    QLabel*                             _notSelectedLabel;

    UavWidget*                          _currentVehicleWidget;
    UavsDialog*                         _vehiclesDialog;

    bool                                _isAllowAddVehicle;

    Q_DISABLE_COPY(SelectedUavWidget)
};
