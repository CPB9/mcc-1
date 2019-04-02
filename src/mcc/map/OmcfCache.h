#pragma once

#include "mcc/map/FileCache.h"
#include "mcc/geo/MercatorProjection.h"
#include "mcc/map/TilePosCache.h"
#include "mcc/map/Rc.h"

#include <bmcl/Fwd.h>

#include <QString>
#include <QFile>

#include <cstdint>

class QImage;

namespace mccmap {

struct TilePosition;

class MCC_MAP_DECLSPEC OmcfCache : public FileCache {
public:
    static constexpr uint32_t headerMagic = 0x5a5a5a5a;

    enum Result { Ok, NoFilesFound, WriteError };
    ~OmcfCache();

    static uint32_t crc32(const void* data, std::size_t len);

    Result open(const QString& path);

    static bmcl::Result<Rc<OmcfCache>, Result> create(const QString& path);

    inline int size() const;

    const mccgeo::MercatorProjection& projection() const override;
    const QString& description() const override;
    const QString& name() const override;
    QImage readImage(const TilePosition& pos, const QRect& rect) const override;
    bool tileExists(const TilePosition& pos) const override;

    const QString& path() const;

private:
    struct TileFileInfo {
        int64_t offset;
        int64_t size;
    };

    FastTilePosCache<TileFileInfo> _cache;
    mccgeo::MercatorProjection _proj;
    QFile _file;
    QString _name;
    QString _description;
    QString _path;
    const uchar* _mapped;
};

inline int OmcfCache::size() const
{
    return (int)_cache.count();
}
}
