#pragma once

#include "mcc/map/FileCache.h"

#include <QString>

class QImage;

namespace mccmap {

struct TilePosition;

class MCC_MAP_DECLSPEC DiskCache : public FileCache {
public:
    DiskCache(const QString& cachePath, const QString& subdir = QString(), const char* format = "jpg");
    ~DiskCache();

    bmcl::Option<QString> generateTileSavePath(const mccmap::TilePosition & pos) override;
    QImage readImage(const TilePosition& pos, const QRect& rect) const override;
    bool tileExists(const TilePosition& pos) const override;

    void setPath(const QString& basePath, const QString& subdir = QString());
    const QString& path() const;

private:
    QString _cachePath;
};
}
