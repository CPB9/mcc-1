#pragma once

#include "mcc/Config.h"
#include "mcc/uav/Fwd.h"
#include "mcc/uav/Rc.h"
#include "mcc/ui/Fwd.h"
#include "mcc/uav/Route.h"
#include "mcc/uav/Uav.h"
#include "mcc/ui/Dialog.h"
#include "mcc/ui/CoordinateSystemController.h"

#include <QDoubleSpinBox>

class QLineEdit;
class QComboBox;
class QLabel;
class QCheckBox;
class QPushButton;
class QGroupBox;
class QButtonGroup;
class QRadioButton;
class QStackedWidget;
class QGridLayout;
class QMenu;

namespace mccuav {

class MCC_UAV_DECLSPEC WaypointSettings : public mccui::Dialog
{
    Q_OBJECT
public:
    WaypointSettings(mccuav::Uav* device,
                     mccuav::Route* route,
                     const mccui::CoordinateSystemController* csController,
                     mccui::Settings* settings,
                     QWidget* parent = nullptr);
    ~WaypointSettings() override;

    void set(int inx);
    void setEditMode(mccuav::Route::EditMode editMode);
protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;

    virtual mccgeo::LatLon initialLatLon() const = 0;
    virtual double nextPointDistance() const = 0;
    virtual double angleBetween(const mccgeo::LatLon& p1, const mccgeo::LatLon& p2) const = 0;
    virtual mccgeo::LatLon latLonOnLine(const mccgeo::LatLon& p, double angle, double offset) const = 0;
public slots:
    void apply();
private slots:
    void addPropertyClicked();
    void removePropertyClicked();
signals:
    void okClicked();
    void cancelClicked();

private:
    void setCorrectHeight();
    void updateAvailableFlags();
    void updateProperties();

    QLabel*                         _routeName;
    QLabel*                         _pointIndex;

    mccui::FastEditDoubleSpinBox*   _speedBox;
    QComboBox*                      _speedUnits;
    QPushButton*                    _speedToAll;
    mccui::FastEditDoubleSpinBox*   _heightBox;
    QComboBox*                      _heightUnits;
    QComboBox*                      _heightTypeBox;
    QPushButton*                    _heightToAll;
    mccui::LatLonEditor*            _latLonEditor;
    QPushButton*                    _addButton;
    QPushButton*                    _removeButton;
    QPushButton*                    _uploadButton;
    QPushButton*                    _okButton;

    QGroupBox*                           _flagsGroupBox;
    std::vector<bmcl::Rc<mccmsg::PropertyEditor>> _propertiesEditors;
    QGridLayout*                         _flagsLayout;
    QPushButton*                         _addPropertyButton;
    QMenu*                               _addPropertyMenu;

    mccui::FastEditDoubleSpinBox*   _sleepBox;
    QComboBox*                      _sleepUnits;
    QButtonGroup*                   _modeGroup;
    QRadioButton*                   _noModeButton;
    QRadioButton*                   _reynoldsModeButton;
    QRadioButton*                   _formationModeButton;
    QRadioButton*                   _snakeModeButton;
    QRadioButton*                   _loopModeButton;

    QStackedWidget*                 _modeDetailsWidget;
    mccui::FastEditDoubleSpinBox*   _loopRadius;

    mccuav::Uav*                    _uav;
    mccuav::Route*                  _route;
    int                             _idx;
    bool                            _updateFlags;

    Rc<const mccui::CoordinateSystemController> _csController;
    Rc<mccui::Settings>             _settings;
    bool                            _aerobotMode;
};
}
