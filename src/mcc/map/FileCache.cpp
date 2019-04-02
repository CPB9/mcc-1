#include "mcc/map/FileCache.h"
#include "mcc/map/TilePosition.h"
#include "mcc/map/WebMapProperties.h"

#include <bmcl/Option.h>

#include <QRect>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QString>
#include <QImage>

#include <cmath>

namespace mccmap {

FileCache::FileCache(const char* format)
    : _format(format)
{
}

FileCache::~FileCache()
{
}

QString FileCache::createPath(const TilePosition& pos, const char* format)
{
    QChar sep = '/';
    QString path;
    path.reserve(52); // достаточно - 52 один из размеров роста QString
    path.append('z');
    path.append(QString::number(pos.zoomLevel + 1));
    path.append(sep);
    path.append(QString::number(pos.globalOffsetX / 1024));
    path.append(sep);
    path.append('x');
    path.append(QString::number(pos.globalOffsetX));
    path.append(sep);
    path.append(QString::number(pos.globalOffsetY / 1024));
    path.append(sep);
    path.append('y');
    path.append(QString::number(pos.globalOffsetY));
    path.append('.');
    path.append(format);
    return path;
}

static int parse(const QString& str, char c, int min, int max)
{
    if (str[0] != c) {
        return -1;
    }
    bool isOk;
    int zoom = str.midRef(1).toInt(&isOk);
    if (!isOk || zoom < min || zoom > max) {
        return -1;
    }
    return zoom;
}

static int parseXY(const QString& subDirStr, const QString& xyStr, char c, int min, int max)
{
    bool isOk;
    int xySubDir = subDirStr.toInt(&isOk);
    if (!isOk || xySubDir < 0) {
        return -1;
    }

    int xy = parse(xyStr, c, min, max);
    if (xy < 0 || ((xy / 1024) != xySubDir)) {
        return -1;
    }
    return xy;
}

bmcl::Option<TilePosition> FileCache::createPosition(const QString& relativePath, const char* format)
{
    QChar sep = '/'; // все пути созадаемые в qt используют разделитель /
    QStringList subStrs = relativePath.split(sep, QString::SkipEmptyParts);
    if (subStrs.length() != 5) {
        return bmcl::None;
    }

    int zoom = parse(subStrs[0], 'z', WebMapProperties::minZoom(), WebMapProperties::maxZoom());
    if (zoom < 1) {
        return bmcl::None;
    }
    zoom--; //backwards compatible

    int maxTileIndex = std::exp2(zoom) - 1;
    int x = parseXY(subStrs[1], subStrs[2], 'x', 0, maxTileIndex);
    if (x < 0) {
        return bmcl::None;
    }

    QStringList ySubStrs = subStrs[4].split('.', QString::SkipEmptyParts);
    if (ySubStrs.length() == 2) {
        if (ySubStrs[1] != format) {
            return bmcl::None;
        }
    } else {
        return bmcl::None;
    }
    int y = parseXY(subStrs[3], ySubStrs[0], 'y', 0, maxTileIndex);
    if (y < 0) {
        return bmcl::None;
    }

    return TilePosition(zoom, x, y);
}

std::pair<QImage, FileCache::TileType> FileCache::loadTile(const TilePosition& pos) const
{
    QRect rect = QRect(0, 0, 256, 256);
    TilePosition newPos = pos;
    int relativeZoom = 1;

    while (newPos.zoomLevel > 0 && !tileExists(newPos)) {
        if (relativeZoom <= 128) {
            relativeZoom *= 2;
        }
        int offsetX = pos.globalOffsetX % relativeZoom;
        int offsetY = pos.globalOffsetY % relativeZoom;
        int size = 256 / relativeZoom;
        rect = QRect(offsetX * size, offsetY * size, size, size);
        newPos.zoomLevel--;
        newPos.globalOffsetX /= 2;
        newPos.globalOffsetY /= 2;
    }

    QImage img = readImage(newPos, rect);
    if (img.isNull()) {
        return std::pair<QImage, TileType>(std::move(img), FileCache::Empty);
    }
    if (relativeZoom == 1) {
        return std::pair<QImage, TileType>(std::move(img), FileCache::Original);
    }
    QImage scaled = img.scaled(256, 256, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    return std::pair<QImage, TileType>(std::move(scaled), FileCache::Scaled);
}

bool FileCache::isBuiltIn() const
{
    return false;
}

int FileCache::maxTileZoom() const
{
    return WebMapProperties::maxZoom();
}

bmcl::Option<std::string> FileCache::generateTileUrl(const TilePosition& pos)
{
    return bmcl::None;
}

bmcl::Option<QString> FileCache::generateTileSavePath(const TilePosition& pos)
{
    return bmcl::None;
}

bool FileCache::hasOnlineTiles() const
{
    return false;
}

bmcl::Option<TilePosition> FileCache::createPosition(const QString& relativePath) const
{
    return createPosition(relativePath, _format);
}

QString FileCache::createPath(const TilePosition& pos) const
{
    return createPath(pos, _format);
}

EmptyFileCache::EmptyFileCache()
{
}

EmptyFileCache::~EmptyFileCache()
{
}

std::pair<QImage, FileCache::TileType> EmptyFileCache::loadTile(const TilePosition& pos) const
{
    return {QImage(), FileCache::Empty};
}

bool EmptyFileCache::isBuiltIn() const
{
    return false;
}

int EmptyFileCache::maxTileZoom() const
{
    return 0;
}

bmcl::Option<std::string> EmptyFileCache::generateTileUrl(const TilePosition& pos)
{
    return bmcl::None;
}

bmcl::Option<QString> EmptyFileCache::generateTileSavePath(const TilePosition& pos)
{
    return bmcl::None;
}

bool EmptyFileCache::hasOnlineTiles() const
{
    return false;
}

const mccgeo::MercatorProjection& EmptyFileCache::projection() const
{
    return _proj;
}

const QString& EmptyFileCache::description() const
{
    return _empty;
}

const QString& EmptyFileCache::name() const
{
    return _empty;
}

QImage EmptyFileCache::readImage(const TilePosition& pos, const QRect& rect) const
{
    return QImage();
}

bool EmptyFileCache::tileExists(const TilePosition& pos) const
{
    return false;
}
}
