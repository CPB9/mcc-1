#pragma once

#include <QWidget>

#include "mcc/Config.h"
#include "mcc/ui/CoordinateSystemController.h"
#include "mcc/ui/Rc.h"
#include "mcc/geo/LatLon.h"
#include "mcc/geo/Position.h"

#include <bmcl/Option.h>

class QComboBox;
class QLabel;
class QLineEdit;
class QToolButton;

namespace mccui {

class CoordinateEditor;
class CoordinateSystemController;

class MCC_UI_DECLSPEC LatLonEditor : public QWidget
{
    Q_OBJECT

public:
    enum class EditMode
    {
        Latitude,
        Longitude,
    };

    LatLonEditor(const CoordinateSystemController* csController, QWidget* parent = 0);
    ~LatLonEditor();

    mccgeo::LatLon latLon() const;
    mccgeo::Position positionWithConvertedAlt(double alt) const;
    bool isAltitudeVisible() const;
    bool isEditable() const;

    AngularFormat angularFormat() const;
    const CoordinateSystem& currentSystem() const;

    void resetFormat();

    static double normalizeAngle180Deg(double angleDeg);
    static double normalizeAngle360Deg(double angleDeg);

signals:
    void valueChanged(const mccgeo::LatLon& latLon);
    void systemChanged();

public slots:
    void setLatLon(mccgeo::LatLon latLon);
    void setEditMode(EditMode mode);
    void setAltitudeVisible(bool visible);
    void setEditable(bool editable);

private slots:
    void onSystemChanged();
    void onAngularFormatChanged();
    void copy();
    void paste();

private:
    void setCoordinateSystem(std::size_t system);

    CoordinateEditor*       _latEditor;
    CoordinateEditor*       _lonEditor;
    QLineEdit*              _altEditor;
    QLabel*                 _latLabel;
    QLabel*                 _lonLabel;
    QLabel*                 _altLabel;

    QComboBox*              _systemSelector;
    QComboBox*              _formatSelector;
    QToolButton*            _copyButton;
    QToolButton*            _pasteButton;

    mccgeo::Position _convertedPosition;

    Rc<const CoordinateSystemController> _csController;
    std::size_t _systemIndex;

    Q_DISABLE_COPY(LatLonEditor)
};
}
