#pragma once

#include "mcc/Config.h"
#include "mcc/geo/Fwd.h"
#include "mcc/geo/MercatorProjection.h"
#include "mcc/map/Rc.h"
#include "mcc/ui/QObjectRefCountable.h"

#include <bmcl/Fwd.h>

#include <QString>
#include <QObject>

#include <string>
#include <utility>

namespace bmcl {
class Buffer;
}

class QImage;
class QRect;
class QString;

namespace mccmap {

struct TilePosition;

class MCC_MAP_DECLSPEC FileCache : public mccui::QObjectRefCountable<QObject> {
    Q_OBJECT
public:
    enum TileType { Original, Scaled, Empty };
    FileCache(const char* format = "jpg");
    virtual ~FileCache();

    static QString createPath(const TilePosition& pos, const char* format);
    static bmcl::Option<TilePosition> createPosition(const QString& relativePath, const char* format);

    QString createPath(const TilePosition& pos) const;
    bmcl::Option<TilePosition> createPosition(const QString& relativePath) const;
    virtual std::pair<QImage, FileCache::TileType> loadTile(const TilePosition& pos) const;
    inline const char* format() const;
    inline void setFormat(const char* format);

    virtual bool isBuiltIn() const;
    virtual int maxTileZoom() const;
    virtual bmcl::Option<std::string> generateTileUrl(const TilePosition& pos);
    virtual bmcl::Option<QString> generateTileSavePath(const TilePosition& pos);
    virtual bool hasOnlineTiles() const;
    virtual const mccgeo::MercatorProjection& projection() const = 0;
    virtual const QString& description() const = 0;
    virtual const QString& name() const = 0;
    virtual QImage readImage(const TilePosition& pos, const QRect& rect) const = 0;
    virtual bool tileExists(const TilePosition& pos) const = 0;

signals:
    void cacheReloaded();

private:
    const char* _format;
};

inline const char* FileCache::format() const
{
    return _format;
}

inline void FileCache::setFormat(const char* format)
{
    _format = format;
}

class MCC_MAP_DECLSPEC EmptyFileCache : public FileCache {
public:
    EmptyFileCache();
    virtual ~EmptyFileCache() override;

    std::pair<QImage, FileCache::TileType> loadTile(const TilePosition& pos) const override;
    bool isBuiltIn() const override;
    int maxTileZoom() const override;
    bmcl::Option<std::string> generateTileUrl(const TilePosition& pos) override;
    bmcl::Option<QString> generateTileSavePath(const TilePosition& pos) override;
    bool hasOnlineTiles() const override;
    const mccgeo::MercatorProjection& projection() const override;
    const QString& description() const override;
    const QString& name() const override;
    QImage readImage(const TilePosition& pos, const QRect& rect) const override;
    bool tileExists(const TilePosition& pos) const override;

private:
    QString _empty;
    mccgeo::MercatorProjection _proj;
};
}
