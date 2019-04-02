#pragma once

#include "mcc/Config.h"
#include "mcc/map/FileCache.h"
#include "mcc/geo/MercatorProjection.h"
#include "mcc/map/Rc.h"

#include <vector>

namespace mccmap {

class MCC_MAP_DECLSPEC StackCache : public FileCache {
public:
    StackCache();
    ~StackCache();
    static Rc<StackCache> create();

    const FileCache* at(std::size_t index) const;
    void append(const FileCache* cache, bool isEnabled = true);
    void prepend(const FileCache* cache, bool isEnabled = true);
    void insertAt(const FileCache* cache, std::size_t index, bool isEnabled = true);
    void removeAt(std::size_t index);
    void setEnabled(std::size_t index, bool flag);
    bool isEnabled(std::size_t index) const;
    std::size_t size() const;
    void swap(std::size_t left, std::size_t right);
    void setProjection(const mccgeo::MercatorProjection& proj);
    void clear();
    void move(std::size_t srcFirst, std::size_t count, std::size_t destFirst);

    const mccgeo::MercatorProjection& projection() const override;
    const QString& description() const override;
    const QString& name() const override;
    QImage readImage(const TilePosition& pos, const QRect& rect) const override;
    bool tileExists(const TilePosition& pos) const override;

    bool hasOnlineCache(const std::string& fullName) const;

private:
    struct CacheElement {
        CacheElement(const FileCache* cache, bool isEnabled)
            : cache(cache)
            , isEnabled(isEnabled)
        {
        }
        Rc<const FileCache> cache;
        bool isEnabled;
    };
    std::vector<CacheElement> _caches;
    mccgeo::MercatorProjection _proj;
    QString _desc;
};
}
