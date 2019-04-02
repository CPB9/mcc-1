#pragma once

#include "mcc/Config.h"
#include "mcc/geo/Fwd.h"
#include "mcc/geo/CoordinateConverter.h"
#include "mcc/ui/Rc.h"
#include "mcc/ui/QObjectRefCountable.h"
#include "mcc/ui/CoordinateFormat.h"
#include "mcc/ui/CoordinateFormatter.h"
#include "mcc/plugin/PluginData.h"

#include <bmcl/Either.h>

#include <QObject>
#include <QString>
#include <QMimeData>
#include <QRegExp>

#include <utility>

namespace mccui {

class Settings;
class SettingsWriter;
class CoordinateSystem;

class MCC_UI_DECLSPEC CoordinateSystemController : public QObjectRefCountable<QObject> {
    Q_OBJECT

public:
    CoordinateSystemController(Settings* settings);
    ~CoordinateSystemController();

    CoordinateFormatter::Formatted2Dim convertAndFormat(mccgeo::LatLon coord) const;
    CoordinateFormatter::Formatted3Dim convertAndFormat(const mccgeo::Position& coord) const;
    CoordinateFormatter::Formatted4Dim convertAndFormat(const mccgeo::Coordinate& coord) const;

    QString convertAndFormat(mccgeo::LatLon coord, const QString& format) const;
    QString convertAndFormat(const mccgeo::Position& coord, const QString& format) const;
    QString convertAndFormat(const mccgeo::Coordinate& coord, const QString& format) const;

    //HACK
    static QString formatValue(double value, const CoordinateFormat& fmt, int prec = 8);

    void setAngularFormat(AngularFormat format);
    AngularFormat angularFormat() const;

    const CoordinateFormat& format() const; //xy
    const CoordinateFormat& vformat() const; //z

    bool selectSystem(std::size_t index);
    std::size_t currentSystemIndex() const;
    const CoordinateSystem& currentSystem() const;
    const mccgeo::CoordinateConverter* currentConverter() const;

    const std::vector<CoordinateSystem>& systems() const;

    QMimeData* makeMimeData(mccgeo::LatLon coord) const;
    QMimeData* makeMimeData(const mccgeo::Position& coord) const;
    QMimeData* makeMimeData(const mccgeo::Coordinate& coord) const;

    MCC_DELETE_COPY_MOVE_CONSTRUCTORS(CoordinateSystemController)

    //TODO: move into math
    static int decomposeDegree(double inputDeg, double* outputMin);

signals:
    void angularFormatChanged(AngularFormat format);
    void coordinateSystemSelected(std::size_t index, const CoordinateSystem& system);
    void coordinateSystemAdded(std::size_t index, const CoordinateSystem& system);

    void changed();

private:
    std::vector<CoordinateSystem> _descriptors;
    std::size_t _currentSystem;
    CoordinateFormatter _formatter;
    AngularFormat _angularFormat;
    Rc<SettingsWriter> _angularFormatWriter;
    Rc<SettingsWriter> _systemIndexWriter;
};

class MCC_UI_DECLSPEC CoordinateSystemControllerPluginData : public mccplugin::PluginData {
public:
    static constexpr const char* id = "mcc::CoordinateSystemControllerPluginData";

    CoordinateSystemControllerPluginData(CoordinateSystemController* settings);
    ~CoordinateSystemControllerPluginData();

    CoordinateSystemController* csController();
    const CoordinateSystemController* csController() const;

private:
    Rc<CoordinateSystemController> _controller;
};
}
