#pragma once

#include <QObject>
#include <QPixmap>

#include <bmcl/Rc.h>

#include "mcc/Config.h"
#include "mcc/geo/LatLon.h"

#include "mcc/ui/GraphicsEffectCreator.h"
#include "mcc/ui/QObjectRefCountable.h"

namespace mccuav {

class MCC_UAV_DECLSPEC PointOfInterest : public mccui::QObjectRefCountable<QObject>
{
    Q_OBJECT
    Q_PROPERTY(QString info      READ info         WRITE setInfo         NOTIFY infoChanged)
    Q_PROPERTY(int icon          READ icon         WRITE setIcon         NOTIFY pixmapChanged)
    Q_PROPERTY(double latitude   READ latitude     WRITE setLatitude     NOTIFY latitudeChanged)
    Q_PROPERTY(double longitude  READ longitude    WRITE setLongitude    NOTIFY longitudeChanged)
    Q_PROPERTY(double altitude   READ altitude     WRITE setAltitude     NOTIFY altitudeChanged)
    Q_PROPERTY(double azimuth    READ azimuth      WRITE setAzimuth      NOTIFY azimuthChanged)
    Q_PROPERTY(double elevation  READ elevation    WRITE setElevation    NOTIFY elevationChanged)
    Q_PROPERTY(bool useAzimuth   READ useAzimuth   WRITE setUseAzimuth   NOTIFY useAzimuthChanged)
    Q_PROPERTY(bool useElevation READ useElevation WRITE setUseElevation NOTIFY useElevationChanged)
    Q_PROPERTY(bool readOnly     READ isEditable     WRITE setEditable     NOTIFY editableChanged)
    Q_PROPERTY(bool visible      READ isVisible      WRITE setVisible      NOTIFY visibleChanged)
public:
    enum Type
    {
            Runway,               // взлетно-посадочная полоса
            Target,               // перекрестие прицела
            Parachute,            // парашют
            ParachuteWithPayload, // парашют с грузом
            Bomb,                 // бомба
            Missile,              // ракета
            CruiseMissile,        // крылатая ракета
            Antenna,              // Антенна
    };

    PointOfInterest();
    PointOfInterest(const QString& info, Type type, const mccgeo::LatLon& latLon);
    ~PointOfInterest();

    void setIcon(int iconType);
    int icon() const;

    const QPixmap   activePixmap() const;
    const QPixmap   inactivePixmap() const;
    void            createPixmaps();
    void setBaseColor(const QColor& color);
    const QString&  info() const;
    bool            useAzimuth() const;
    bool            useElevation() const;

    bool            isEditable() const;
    bool            isVisible() const;

    double          latitude();
    double          longitude();
    double          altitude();
    double          azimuth();
    double          elevation();

    void            setLatitude(double lat);
    void            setLongitude(double lon);
    void            setAltitude(double alt);
    void            setAzimuth(double azimuth);
    void            setElevation(double elevation);
    void            setUseAzimuth(bool use);
    void            setUseElevation(bool use);
    void            setInfo(const QString& info);
    void            setEditable(bool editable);
    void            setVisible(bool visible);

    void            setUseBaseColor(bool use);
    const mccgeo::LatLon& latLon() const;
    mccgeo::LatLon& latLon();

    void            setLatLon(const mccgeo::LatLon& latLon);

    Q_INVOKABLE void updateFinished();
    Type kind() const { return _kind; }

    void startMoving();
    void finishMoving();

    bool isMoving() const;
    bool updatePending() const;
public slots:
    void setEnabled(bool isEnabled);

signals:
    void azimuthChanged();
    void elevationChanged();
    void latLonChanged();
    void uploadToUav();
    void pixmapChanged();
    void latitudeChanged();
    void longitudeChanged();
    void altitudeChanged();
    void useAzimuthChanged();
    void useElevationChanged();
    void infoChanged();
    void editableChanged();
    void visibleChanged();
    void updated();
    void showEditor();
private:
    double _altitude;
    double _azimuth;
    double _elevation;
    bool _useAzimuth;
    bool _useAngle;
    bool _editable;
    bool _visible;
    bool _updatePending;
    bool _isEnabled;
    bool _isMoving;

    mccgeo::LatLon _latLon;
    int _icon;
    QPixmap _inactivePixmap;
    QPixmap _activePixmap;
    QPixmap _grayPixmap;
    QColor  _baseColor;
    QString _info;
    Type    _kind;
    bool _useBaseColor;
};

using PointOfInterestPtr = bmcl::Rc<PointOfInterest>;
using PointOfInterestPtrs = std::vector<PointOfInterestPtr>;
}
