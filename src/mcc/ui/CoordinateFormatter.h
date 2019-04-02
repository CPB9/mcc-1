#pragma once

#include "mcc/Config.h"

#include "mcc/ui/CoordinateFormat.h"
#include "mcc/geo/Fwd.h"

#include <bmcl/Fwd.h>

#include <QString>

class QMimeData;

namespace mccui {

class CoordinateSystem;

class MCC_UI_DECLSPEC CoordinateFormatter {
public:
    struct Formatted2Dim {
        QString x;
        QString y;
    };

    struct Formatted3Dim {
        QString x;
        QString y;
        QString z;
    };

    struct Formatted4Dim {
        QString x;
        QString y;
        QString z;
        QString t;
    };

    CoordinateFormatter();
    ~CoordinateFormatter();

    int precision() const;

    const CoordinateFormat& format() const;
    const CoordinateFormat& vformat() const;

    CoordinateFormat& format();
    CoordinateFormat& vformat();

    void setPrecision(int precision);
    void setFormat(const CoordinateFormat& fmt);
    void setVFormat(const CoordinateFormat& vfmt);
    void setFormatsFromSystemAndAngular(const CoordinateSystem& system, AngularFormat angular);

    void updateAngularFormat(AngularFormat angular);

    Formatted2Dim convertAndFormat(const mccgeo::CoordinateConverter* conv, mccgeo::LatLon coord) const;
    Formatted3Dim convertAndFormat(const mccgeo::CoordinateConverter* conv, const mccgeo::Position& coord) const;
    Formatted4Dim convertAndFormat(const mccgeo::CoordinateConverter* conv, const mccgeo::Coordinate& coord) const;

    QString convertAndFormat(const mccgeo::CoordinateConverter* conv, mccgeo::LatLon coord, const QString& format) const;
    QString convertAndFormat(const mccgeo::CoordinateConverter* conv, const mccgeo::Position& coord, const QString& format) const;
    QString convertAndFormat(const mccgeo::CoordinateConverter* conv, const mccgeo::Coordinate& coord, const QString& format) const;

    QMimeData* makeMimeData(const mccgeo::CoordinateConverter* conv, const QString& systemName, mccgeo::LatLon coord) const;
    QMimeData* makeMimeData(const mccgeo::CoordinateConverter* conv, const QString& systemName, const mccgeo::Position& coord) const;
    QMimeData* makeMimeData(const mccgeo::CoordinateConverter* conv, const QString& systemName, const mccgeo::Coordinate& coord) const;

    static bmcl::Option<mccgeo::Coordinate> decodeFromMimeData(const QMimeData* mimeData);

    static int decomposeDegree(double inputDeg, double* outputMin);
    static QString formatValue(double value, const CoordinateFormat& fmt, int prec = 8);

private:
    QMimeData* makeMimeDataFromStringAndCoord(const QString& str, const mccgeo::Coordinate& coord) const;

    CoordinateFormat _format;
    CoordinateFormat _vformat;
    int _precision;
};
}

