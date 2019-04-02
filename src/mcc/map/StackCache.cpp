#include "mcc/map/StackCache.h"
#include "mcc/map/FileCache.h"
#include "mcc/map/OnlineCache.h"

#include <QImage>

namespace mccmap {

StackCache::StackCache()
    : _desc("Стек карт")
{
}

StackCache::~StackCache()
{
}

void StackCache::clear()
{
    _caches.clear();
}

bool StackCache::tileExists(const TilePosition& pos) const
{
    for (const CacheElement& element : _caches) {
        if (element.isEnabled && element.cache->tileExists(pos)) {
            return true;
        }
    }
    return false;
}

QImage StackCache::readImage(const TilePosition& pos, const QRect& rect) const
{
    for (const CacheElement& element : _caches) {
        if (element.isEnabled) {
            QImage img = element.cache->readImage(pos, rect);
            if (!img.isNull()) {
                return img;
            }
        }
    }
    return QImage();
}

const QString& StackCache::description() const
{
    return _desc;
}

const QString& StackCache::name() const
{
    return _desc;
}

const mccgeo::MercatorProjection& StackCache::projection() const
{
    return _proj;
}

bool StackCache::hasOnlineCache(const std::string& fullName) const
{
    for (const CacheElement& element : _caches) {
        if (element.cache->isBuiltIn()) {
            const OnlineCache* onlineCache = static_cast<const OnlineCache*>(element.cache.get());
            if (onlineCache->fullName() == fullName) {
                return true;
            }
        }
    }
    return false;
}

Rc<StackCache> StackCache::create()
{
    return new StackCache();
}

void StackCache::setProjection(const mccgeo::MercatorProjection& proj)
{
    _proj = proj;
}

const FileCache* StackCache::at(std::size_t index) const
{
    return _caches[index].cache.get();
}

void StackCache::append(const FileCache* cache, bool isEnabled)
{
    _caches.emplace_back(cache, isEnabled);
}

void StackCache::prepend(const FileCache* cache, bool isEnabled)
{
    _caches.emplace(_caches.begin(), cache, isEnabled);
}

void StackCache::swap(std::size_t left, std::size_t right)
{
    std::swap(_caches[left], _caches[right]);
}

void StackCache::insertAt(const FileCache* cache, std::size_t index, bool isEnabled)
{
    _caches.emplace(_caches.begin() + index, cache, isEnabled);
}

void StackCache::removeAt(std::size_t index)
{
    _caches.erase(_caches.begin() + index);
}

bool StackCache::isEnabled(std::size_t index) const
{
    return _caches[index].isEnabled;
}

std::size_t StackCache::size() const
{
    return _caches.size();
}

void StackCache::setEnabled(std::size_t index, bool flag)
{
    _caches[index].isEnabled = flag;
}

void StackCache::move(std::size_t srcFirst, std::size_t count, std::size_t destFirst)
{
    //FIXME: std::move(it, it, it)
    std::vector<CacheElement> tmp(_caches.begin() + srcFirst, _caches.begin() + srcFirst + count);
    _caches.insert(_caches.begin() + destFirst, tmp.begin(), tmp.end());
    if (srcFirst < destFirst) {
        _caches.erase(_caches.begin() + srcFirst, _caches.begin() + srcFirst + count);
    } else {
        _caches.erase(_caches.begin() + srcFirst + count, _caches.begin() + srcFirst + count + count);
    }
}
}
