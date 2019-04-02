#include "mcc/ui/CoordinateSystemController.h"
#include "mcc/ui/CoordinateSystem.h"
#include "mcc/ui/CoordinateFormat.h"
#include "mcc/ui/Settings.h"
#include "mcc/geo/Coordinate.h"
#include "mcc/geo/CoordinateConverter.h"

#include <bmcl/Math.h>

#include <QApplication>
#include <QClipboard>
#include <QStringBuilder>

#include <bmcl/Result.h>
#include <bmcl/Logging.h>

#include <sstream>
#include <iomanip>

#include <cmath>
#include <stdint.h>

namespace mccui {

//TODO: refact

struct ProjDef {
    const char* fullName;
    const char* shortName;
    const char* def;
};

static const ProjDef _wgs84Def = {"EPSG:4326 - WGS 84", "WGS84", "+proj=longlat +datum=WGS84 +no_defs"};

static const ProjDef _projDefs[3] = {
    {"EPSG:4284 - Пулково 1942", "СК42", "+proj=longlat +ellps=krass +towgs84=23.92,-141.27,-80.9,-0,0.35,0.82,-0.12 +no_defs"},
    {"EPSG:3395 - Меркатор", "Меркатор", "+proj=merc +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +no_defs "},
    {"EPSG:3857 - Сферический меркатор", "Сф. меркатор", "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext  +no_defs"},
};

static std::string createLocalSystemDef(mccgeo::Position origin, double clockwiseRotationDegrees)
{
    std::stringstream result;
    result << std::setprecision(std::numeric_limits<double>::digits10);
    result << "+ellps=WGS84 +proj=omerc +lat_0=";
    result << origin.latitude();
    result << " +lonc=";
    result << origin.longitude();
    result << " +z_0="; //WARNING: возможно старый proj не тянет смещение по высоте
    result << origin.altitude();
    result << " +alpha=";
    result << clockwiseRotationDegrees;
    result << " +gamma=0 +units=m +no_defs";
    return result.str();
}

CoordinateSystemController::CoordinateSystemController(Settings* settings)
    : _currentSystem(0)
    , _angularFormat(AngularFormat::Degrees)
{
    _angularFormatWriter = settings->acquireUniqueWriter("coordsystem/angularFormat", (int)AngularFormat::Degrees).unwrap();
    _systemIndexWriter = settings->acquireUniqueWriter("coordsystem/currentSystemIndex", 0).unwrap();

    auto wgs84rv = mccgeo::CoordinateConverter::createFromProj4Definition(_wgs84Def.def);
    assert(wgs84rv.isOk());
    _descriptors.emplace_back(_wgs84Def.fullName, _wgs84Def.shortName, wgs84rv.unwrap().get());
    _formatter.setFormatsFromSystemAndAngular(_descriptors[0], _angularFormat);

    constexpr std::size_t numDefs = sizeof(_projDefs) / sizeof(_projDefs[0]);
    for (std::size_t i = 0; i < numDefs; i++) {
        auto rv = mccgeo::CoordinateConverter::createFromProj4Definition(_projDefs[i].def);
        if (rv.isOk()) {
            auto conv = rv.take();
            _descriptors.emplace_back(_projDefs[i].fullName, _projDefs[i].shortName, conv.get());
        } else {
            BMCL_WARNING() << "Failed to init builtin coord system: " << rv.unwrapErr();
        }
    }

    AngularFormat fmt = (AngularFormat)_angularFormatWriter->read().toInt();
    switch(fmt) {
    case AngularFormat::Degrees:
    case AngularFormat::DegreesMinutes:
    case AngularFormat::DegreesMinutesSeconds:
        break;
    default:
        fmt = AngularFormat::Degrees;
    }

    setAngularFormat(fmt);

    std::size_t i = _systemIndexWriter->read().toUInt();
    if (i >= _descriptors.size()) {
        i = 0;
    }
    selectSystem(0);
}

CoordinateSystemController::~CoordinateSystemController()
{
    _angularFormatWriter->write((int)_angularFormat);
    _systemIndexWriter->write((uint)_currentSystem);
}

bool CoordinateSystemController::selectSystem(std::size_t index)
{
    if (index >= _descriptors.size()) {
        return false;
    }
    _currentSystem = index;
    const auto& desc = _descriptors[index];

    _formatter.setFormatsFromSystemAndAngular(desc, _angularFormat);

    emit coordinateSystemSelected(index, desc);
    emit changed();

    return true;
}

QString CoordinateSystemController::formatValue(double value, const CoordinateFormat& fmt, int prec)
{
    return CoordinateFormatter::formatValue(value, fmt, prec);
}

CoordinateFormatter::Formatted2Dim CoordinateSystemController::convertAndFormat(mccgeo::LatLon coord) const
{
    return _formatter.convertAndFormat(currentConverter(), coord);
}

CoordinateFormatter::Formatted3Dim CoordinateSystemController::convertAndFormat(const mccgeo::Position& coord) const
{
    return _formatter.convertAndFormat(currentConverter(), coord);
}

CoordinateFormatter::Formatted4Dim CoordinateSystemController::convertAndFormat(const mccgeo::Coordinate& coord) const
{
    return _formatter.convertAndFormat(currentConverter(), coord);
}

QString CoordinateSystemController::convertAndFormat(mccgeo::LatLon coord, const QString& format) const
{
    return _formatter.convertAndFormat(currentConverter(), coord, format);
}

QString CoordinateSystemController::convertAndFormat(const mccgeo::Position& coord, const QString& format) const
{
    return _formatter.convertAndFormat(currentConverter(), coord, format);
}

QString CoordinateSystemController::convertAndFormat(const mccgeo::Coordinate& coord, const QString& format) const
{
    return _formatter.convertAndFormat(currentConverter(), coord, format);
}

const mccgeo::CoordinateConverter* CoordinateSystemController::currentConverter() const
{
    return currentSystem().converter();
}

const CoordinateSystem& CoordinateSystemController::currentSystem() const
{
    assert(_currentSystem < _descriptors.size());
    return _descriptors[_currentSystem];
}

void CoordinateSystemController::setAngularFormat(AngularFormat format)
{
    _formatter.updateAngularFormat(format);
    emit angularFormatChanged(format);
    emit changed();
}

AngularFormat CoordinateSystemController::angularFormat() const
{
    return _angularFormat;
}

int CoordinateSystemController::decomposeDegree(double inputDeg, double* outputMin)
{
    return CoordinateFormatter::decomposeDegree(inputDeg, outputMin);
}

CoordinateSystemControllerPluginData::CoordinateSystemControllerPluginData(CoordinateSystemController* controller)
    : mccplugin::PluginData(CoordinateSystemControllerPluginData::id)
    , _controller(controller)
{
}

CoordinateSystemControllerPluginData::~CoordinateSystemControllerPluginData()
{
}

CoordinateSystemController* CoordinateSystemControllerPluginData::csController()
{
    return _controller.get();
}

const CoordinateSystemController* CoordinateSystemControllerPluginData::csController() const
{
    return _controller.get();
}

const CoordinateFormat& CoordinateSystemController::format() const
{
    return _formatter.format();
}

const CoordinateFormat& CoordinateSystemController::vformat() const
{
    return _formatter.vformat();
}

std::size_t CoordinateSystemController::currentSystemIndex() const
{
    return _currentSystem;
}

const std::vector<CoordinateSystem>& CoordinateSystemController::systems() const
{
    return _descriptors;
}

QMimeData* CoordinateSystemController::makeMimeData(mccgeo::LatLon coord) const
{
    return _formatter.makeMimeData(currentConverter(), currentSystem().shortName(), coord);
}

QMimeData* CoordinateSystemController::makeMimeData(const mccgeo::Position& coord) const
{
    return _formatter.makeMimeData(currentConverter(), currentSystem().shortName(), coord);
}

QMimeData* CoordinateSystemController::makeMimeData(const mccgeo::Coordinate& coord) const
{
    return _formatter.makeMimeData(currentConverter(), currentSystem().shortName(), coord);
}
}
