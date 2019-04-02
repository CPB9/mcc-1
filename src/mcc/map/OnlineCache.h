#pragma once

#include "mcc/Config.h"
#include "mcc/map/DiskCache.h"
#include "mcc/geo/MercatorProjection.h"
#include "mcc/map/Rc.h"

#include <string>
#include <functional>

namespace mccmap {

struct TilePosition;
class MercatorProjection;
class FileCache;

class MCC_MAP_DECLSPEC OnlineCache : public DiskCache {
public:
    OnlineCache();
    virtual ~OnlineCache();

    template <typename T>
    static Rc<OnlineCache> create();

    bool isBuiltIn() const override;
    int maxTileZoom() const override;
    bmcl::Option<std::string> generateTileUrl(const TilePosition& pos) override;
    bool hasOnlineTiles() const override;
    const mccgeo::MercatorProjection& projection() const override;
    const QString& name() const override;
    const QString& description() const override;

    const std::string& fullName() const;

    void setBasePath(const QString& path);

    virtual const char* createName() const = 0;
    virtual const char* createProvider() const = 0;

    typedef std::function<std::string(const TilePosition&)> Generator;

    virtual int calcMaxTileZoom() = 0;
    virtual const char* createServiceName() const = 0;
    virtual QString createDescription() const = 0;
    virtual const char* createFormat() const = 0;
    virtual mccgeo::MercatorProjection::ProjectionType createProjection() const = 0;
    virtual Generator createGenerator() const = 0;

protected:
    void init();

private:
    mccgeo::MercatorProjection _proj;
    Generator _generator;
    int _maxTileZoom;
    QString _name;
    std::string _fullName;
    QString _description;
};

template <typename T>
Rc<OnlineCache> OnlineCache::create()
{
    Rc<OnlineCache> cache = new T();
    //cache->setPath(mccui::Settings::instance()->mapCachePath(), cache->createServiceName());
    cache->init();
    return cache;
}


class MCC_MAP_DECLSPEC EmptyOnlineCache : public OnlineCache {
public:
    bool isBuiltIn() const override;
    int maxTileZoom() const override;
    bmcl::Option<std::string> generateTileUrl(const TilePosition& pos) override;
    bool hasOnlineTiles() const override;

    const char* createName() const override;
    const char* createProvider() const override;
    int calcMaxTileZoom() override;
    const char* createServiceName() const override;
    QString createDescription() const override;
    const char* createFormat() const override;
    mccgeo::MercatorProjection::ProjectionType createProjection() const override;
    Generator createGenerator() const override;
};
}
