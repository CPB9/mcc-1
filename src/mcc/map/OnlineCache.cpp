#include "mcc/map/OnlineCache.h"
#include "mcc/map/DiskCache.h"
#include "mcc/geo/MercatorProjection.h"

#include <bmcl/Option.h>

#include <cassert>
#include <random>

namespace mccmap {

OnlineCache::OnlineCache()
    : DiskCache("")
{
}

OnlineCache::~OnlineCache()
{
}

void OnlineCache::setBasePath(const QString& path)
{
    setPath(path, createServiceName());
}

bool OnlineCache::isBuiltIn() const
{
    return true;
}

bmcl::Option<std::string> OnlineCache::generateTileUrl(const TilePosition& pos)
{
    return _generator(pos);
}

const mccgeo::MercatorProjection& OnlineCache::projection() const
{
    return _proj;
}

int OnlineCache::maxTileZoom() const
{
    return _maxTileZoom;
}

bool OnlineCache::hasOnlineTiles() const
{
    return true;
}

const QString& OnlineCache::description() const
{
    return _description;
}

const QString& OnlineCache::name() const
{
    return _name;
}

const std::string& OnlineCache::fullName() const
{
    return _fullName;
}

void OnlineCache::init()
{
    auto cache = this;
    cache->setFormat(cache->createFormat());
    cache->_maxTileZoom = cache->calcMaxTileZoom();
    cache->_generator = cache->createGenerator();
    cache->_proj = cache->createProjection();
    cache->_fullName = cache->createProvider();
    cache->_fullName.push_back(' ');
    cache->_fullName.append(cache->createName());
    cache->_name = cache->_fullName.c_str();
    cache->_description = cache->createDescription();
}

bool EmptyOnlineCache::isBuiltIn() const
{
    return false;
}

int EmptyOnlineCache::maxTileZoom() const
{
    return 0;
}

bmcl::Option<std::string> EmptyOnlineCache::generateTileUrl(const TilePosition& pos)
{
    return std::string();
}

bool EmptyOnlineCache::hasOnlineTiles() const
{
    return false;
}

const char* EmptyOnlineCache::createName() const
{
    return "_INVALID_";
}

const char* EmptyOnlineCache::createProvider() const
{
    return "_INVALID_";
}

int EmptyOnlineCache::calcMaxTileZoom()
{
    return 0;
}

const char* EmptyOnlineCache::createServiceName() const
{
    return "_INVALID_";
}

QString EmptyOnlineCache::createDescription() const
{
    return QString("_INVALID_");
}

const char* EmptyOnlineCache::createFormat() const
{
    return ".invalid";
}

mccgeo::MercatorProjection::ProjectionType EmptyOnlineCache::createProjection() const
{
    return mccgeo::MercatorProjection::SphericalMercator;
}

static std::string emptyGenerator(const mccmap::TilePosition& pos)
{
    return std::string();
}

OnlineCache::Generator EmptyOnlineCache::createGenerator() const
{
    return emptyGenerator;
}
}
