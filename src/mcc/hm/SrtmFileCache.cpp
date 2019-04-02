#include "mcc/hm/SrtmFileCache.h"
#include "mcc/hm/Gl1File.h"
#include "mcc/hm/Gl30AllFile.h"

#include <bmcl/Option.h>

#include <limits>
#include <algorithm>

namespace mcchm {

SrtmFileCache::SrtmFileCache(const QString& rootPath, std::size_t size)
    : _path(rootPath)
    , _maxSize(std::max<std::size_t>(size, 1))
    , _invalidFile(Gl1File::createInvalid())
    , _gl30AllFile(Gl30AllFile::load(rootPath))
{
}

SrtmFileCache::~SrtmFileCache()
{
}

Gl1FileDesc SrtmFileCache::loadFile(int latIndex, int lonIndex) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = std::find_if(_files.begin(), _files.end(), [latIndex, lonIndex](const Gl1FileDesc& desc) {
        return desc.latIndex == latIndex && desc.lonIndex == lonIndex;
    });
    if (it != _files.end()) {
        return *it;
    }
    if (_files.size() >= _maxSize) {
        _files.pop_back();
    }
    auto rv =  Gl1File::load(_path, latIndex, lonIndex);
    if (rv.isSome()) {
        _files.emplace_front(latIndex, lonIndex, rv.take());
    } else {
        _files.emplace_front(latIndex, lonIndex, _invalidFile);
    }
    return _files.front();
}

void SrtmFileCache::resize(std::size_t size)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (size > _maxSize) {
        _files.resize(std::max<std::size_t>(size, 1));
        _files.shrink_to_fit();
    }
    _maxSize = size;
}

void SrtmFileCache::setPath(const QString& path)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _path = path;
    _files.clear();
    _gl30AllFile = Gl30AllFile::load(path);
}

const bmcl::OptionRc<const Gl30AllFile>& SrtmFileCache::gl30AllFile() const
{
    return _gl30AllFile;
}
};
