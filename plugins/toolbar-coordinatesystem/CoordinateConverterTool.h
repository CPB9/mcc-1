#pragma once

#include "mcc/geo/Position.h"
#include "mcc/uav/Fwd.h"
#include "mcc/ui/Dialog.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/Rc.h"

#include <bmcl/Option.h>
#include <QWidget>

class CoordinateSettingsPage;

class QAction;
class QComboBox;
class QDoubleSpinBox;
class QGridLayout;
class QLabel;
class QLineEdit;
class QToolButton;

class CoordinateConverterTool : public mccui::Dialog
{
    Q_OBJECT

public:
    explicit CoordinateConverterTool(const mccui::CoordinateSystemController* csController,
                                     mccuav::GlobalActions* actions,
                                     CoordinateSettingsPage* settings,
                                     QWidget* parent = nullptr);
    ~CoordinateConverterTool() override;

public slots:
    void copyToClipboard();
    void pasteFromClipboard();
    void updateSettings();
    void setAltitudeVisible(bool visible);
    void setShortNames(bool shortNames = true);
    void changeOrder();

private slots:
    void handleFirstValues();
    void handleSecondValues();
    void updateUserFormat();

private:
    void setFormats(QComboBox *comboBox);
    void recalculateCoordinates(bool firstToSecond = true);
    void recalculateFirstCoordinates();
    void recalculateSecondCoordinates();
    mccgeo::Position convertPosition(const mccgeo::Position &fromPosition, int fromSystem, int toSystem);
    bmcl::Option<double> parseCoordinate(const QString &coordinate);

private:
    mccui::Rc
    <const mccui::CoordinateSystemController>   _csController;
    mccui::Rc<mccuav::GlobalActions>            _actions;
    CoordinateSettingsPage*                     _settings;

    mccgeo::Position                            _firstPosition;
    mccgeo::Position                            _secondPosition;

    QGridLayout*                                _mainLayout;
    QToolButton*                                _orderButton;
    mccui::CoordinateEditor*                    _firstLatitude;
    mccui::CoordinateEditor*                    _firstLongitude;
    QDoubleSpinBox*                             _firstAltitude;
    QLabel*                                     _latitudeLabel;
    QLabel*                                     _longitudeLabel;
    QLabel*                                     _altitudeLabel;
    mccui::CoordinateEditor*                    _secondLatitude;
    mccui::CoordinateEditor*                    _secondLongitude;
    QDoubleSpinBox*                             _secondAltitude;

    QComboBox*                                  _firstSystem;
    QComboBox*                                  _secondSystem;
    QComboBox*                                  _inputFormat;
    QToolButton*                                _pasteButton;
    QToolButton*                                _copyButton;
    QAction*                                    _actionCopyD;
    QAction*                                    _actionCopyDm;
    QAction*                                    _actionCopyDms;

    bool                                        _inCalculating;  // TODO: refactor to Locker class
    bool                                        _isLatitudeFirst;

    Q_DISABLE_COPY(CoordinateConverterTool)
};
