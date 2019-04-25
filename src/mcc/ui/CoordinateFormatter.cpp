#include "mcc/ui/CoordinateFormatter.h"

#include "mcc/ui/CoordinateSystem.h"
#include "mcc/geo/CoordinateConverter.h"
#include "mcc/geo/Coordinate.h"

#include <bmcl/Option.h>

#include <QMimeData>

#include <cstring>

namespace mccui {

CoordinateFormatter::CoordinateFormatter()
    : _format("")
    , _vformat("")
    , _precision(8)
    , _vPrecision(6)
{
}

CoordinateFormatter::~CoordinateFormatter()
{
}

int CoordinateFormatter::precision() const
{
    return _precision;
}

int CoordinateFormatter::vPrecision() const
{
    return _vPrecision;
}

const CoordinateFormat& CoordinateFormatter::format() const
{
    return _format;
}

const CoordinateFormat& CoordinateFormatter::vformat() const
{
    return _vformat;
}

CoordinateFormat& CoordinateFormatter::format()
{
    return _format;
}

CoordinateFormat& CoordinateFormatter::vformat()
{
    return _vformat;
}

void CoordinateFormatter::setPrecision(int precision)
{
    _precision = precision;
}

void CoordinateFormatter::setVPrecision(int precision)
{
    _vPrecision = precision;
}

void CoordinateFormatter::setFormat(const CoordinateFormat& fmt)
{
    _format = fmt;
}

void CoordinateFormatter::setVFormat(const CoordinateFormat& vfmt)
{
    _vformat = vfmt;
}

void CoordinateFormatter::setFormatsFromSystemAndAngular(const CoordinateSystem& system, AngularFormat angular)
{
    if (system.converter()->units() == mccgeo::CoordinateUnits::Degree) {
        _format.setAngular(angular);
    } else {
        _format.setLinear(system.units());
    }

    if (system.converter()->vunits() == mccgeo::CoordinateUnits::Degree) {
        _vformat.setAngular(angular);
    } else {
        _vformat.setLinear(system.vunits());
    }
}

void CoordinateFormatter::updateAngularFormat(AngularFormat angular)
{
    if (_format.isAngular()) {
        _format.setAngular(angular);
    }
    if (_vformat.isAngular()) {
        _vformat.setAngular(angular);
    }
}

int CoordinateFormatter::decomposeDegree(double inputDeg, double* outputMin)
{
    double deg;
    double min = std::modf(inputDeg, &deg);
    *outputMin = std::abs(min * 60);
    return int(deg);
}

QString CoordinateFormatter::formatValue(double value, const CoordinateFormat& fmt, int prec)
{
    if (fmt.isAngular()) {
        switch (fmt.unwrapAngular()) {
        case AngularFormat::Degrees:
            return QString::number(value, 'f', prec) + "°";
        case AngularFormat::DegreesMinutes: {
            if (prec >= 2) {
                prec -= 2;
            }
            double frac;
            int degrees = decomposeDegree(value, &frac);
            return QString::number(degrees) + "°" + QString::number(frac, 'f', prec) + '\'';
        }
        case AngularFormat::DegreesMinutesSeconds:
            if (prec >= 4) {
                prec -= 4;
            }
            double frac;
            int degrees = decomposeDegree(value, &frac);
            int mins = decomposeDegree(frac, &frac);
            return QString::number(degrees) + "°" + QString::number(mins) + '\'' + QString::number(frac, 'f', prec) + '"';
        }
    } else {
        return QString::number(value, 'f', prec) + fmt.unwrapLinear();
    }
    return QString::number(value, 'f', prec);
}

CoordinateFormatter::Formatted2Dim CoordinateFormatter::convertAndFormat(const mccgeo::CoordinateConverter* conv, mccgeo::LatLon coord) const
{
    mccgeo::Coordinate to = conv->convertForward(coord);
    double x;
    double y;
    if (_format.isAngular()) {
        x = to.y();
        y = to.x();
    } else {
        x = to.x();
        y = to.y();
    }
    return {formatValue(x, _format, _precision), formatValue(y, _format, _precision)};
}

CoordinateFormatter::Formatted3Dim CoordinateFormatter::convertAndFormat(const mccgeo::CoordinateConverter* conv, const mccgeo::Position& coord) const
{
    mccgeo::Coordinate to = conv->convertForward(coord);
    double x;
    double y;
    if (_format.isAngular()) {
        x = to.y();
        y = to.x();
    } else {
        x = to.x();
        y = to.y();
    }
    return {formatValue(x, _format, _precision),
            formatValue(y, _format, _precision),
            formatValue(to.z(), _vformat, _vPrecision)};
}

CoordinateFormatter::Formatted4Dim CoordinateFormatter::convertAndFormat(const mccgeo::CoordinateConverter* conv, const mccgeo::Coordinate& coord) const
{
    mccgeo::Coordinate to = conv->convertForward(coord);
    double x;
    double y;
    if (_format.isAngular()) {
        x = to.y();
        y = to.x();
    } else {
        x = to.x();
        y = to.y();
    }
    return {
        formatValue(x, _format, _precision),
        formatValue(y, _format, _precision),
        formatValue(to.z(), _vformat, _vPrecision),
        formatValue(to.t(), CoordinateFormat("")),
    };
}

QString CoordinateFormatter::convertAndFormat(const mccgeo::CoordinateConverter* conv, mccgeo::LatLon coord, const QString& format) const
{
    Formatted2Dim strs = convertAndFormat(conv, coord);
    return format.arg(strs.x, strs.y);
}

QString CoordinateFormatter::convertAndFormat(const mccgeo::CoordinateConverter* conv, const mccgeo::Position& coord, const QString& format) const
{
    Formatted3Dim strs = convertAndFormat(conv, coord);
    return format.arg(strs.x, strs.y, strs.z);
}

QString CoordinateFormatter::convertAndFormat(const mccgeo::CoordinateConverter* conv, const mccgeo::Coordinate& coord, const QString& format) const
{
    Formatted4Dim strs = convertAndFormat(conv, coord);
    return format.arg(strs.x, strs.y, strs.z, strs.t);
}

static const char* mimeStr = "mcc/coordinates";

bmcl::Option<mccgeo::Coordinate> CoordinateFormatter::decodeFromMimeData(const QMimeData* mimeData)
{
    QByteArray array = mimeData->data(mimeStr);
    if (array.size() != sizeof(mccgeo::Coordinate)) {
        return bmcl::None;
    }

    //HACK: not portable
    mccgeo::Coordinate coord(0, 0);
    std::memcpy(&coord, array.data(), array.size());
    return coord;
}

QMimeData* CoordinateFormatter::makeMimeDataFromStringAndCoord(const QString& str, const mccgeo::Coordinate& coord) const
{
    QMimeData* mimeData = new QMimeData;
    mimeData->setText(str);

    //static_assert(sizeof(coord) == 8*4, "invalid coordinate size");

    //HACK: not portable
    QByteArray encodedArray((const char*)&coord, sizeof(coord));
    mimeData->setData(mimeStr, encodedArray);

    return mimeData;
}

QMimeData* CoordinateFormatter::makeMimeData(const mccgeo::CoordinateConverter* conv, const QString& systemName, mccgeo::LatLon coord) const
{
    Formatted2Dim strs = convertAndFormat(conv, coord);
    QString str = QString("%1, %2 (%3)").arg(strs.x, strs.y, systemName);
    return makeMimeDataFromStringAndCoord(str, coord);
}

QMimeData* CoordinateFormatter::makeMimeData(const mccgeo::CoordinateConverter* conv, const QString& systemName, const mccgeo::Position& coord) const
{
    Formatted3Dim strs = convertAndFormat(conv, coord);
    QString str = QString("%1, %2, %3 (%4)").arg(strs.x, strs.y, strs.z, systemName);
    return makeMimeDataFromStringAndCoord(str, coord);
}

QMimeData* CoordinateFormatter::makeMimeData(const mccgeo::CoordinateConverter* conv, const QString& systemName, const mccgeo::Coordinate& coord) const
{
    Formatted4Dim strs = convertAndFormat(conv, coord);
    QString str = QString("%1, %2, %3, %4 (%5)").arg(strs.x, strs.y, strs.z, strs.t, systemName);
    return makeMimeDataFromStringAndCoord(str, coord);
}
}
