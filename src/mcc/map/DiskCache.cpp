#include "mcc/map/DiskCache.h"

#include <bmcl/Buffer.h>
#include <bmcl/Logging.h>
#include <bmcl/Option.h>

#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QRect>

namespace mccmap {

DiskCache::DiskCache(const QString& cachePath, const QString& subdir, const char* format)
    : FileCache(format)
{
    setPath(cachePath, subdir);
}

void DiskCache::setPath(const QString& basePath, const QString& subdir)
{
    _cachePath = basePath;
    _cachePath.append(QDir::separator());
    _cachePath.append(subdir);
    _cachePath.append(QDir::separator());
}

bool DiskCache::tileExists(const TilePosition& pos) const
{
    return QFileInfo::exists(_cachePath + createPath(pos));
}

QImage DiskCache::readImage(const TilePosition& pos, const QRect& rect) const
{
    QImageReader reader(_cachePath + createPath(pos));
    reader.setClipRect(rect);
    return reader.read();
}

bmcl::Option<QString> DiskCache::generateTileSavePath(const mccmap::TilePosition& pos)
{
    return _cachePath + createPath(pos);
}

DiskCache::~DiskCache()
{
}
}
