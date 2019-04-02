#include "mcc/map/TileLoader.h"
#include "mcc/map/FileCache.h"
#include "mcc/map/CurlEasy.h"

#include <bmcl/Logging.h>
#include <bmcl/Option.h>

#include <QFileInfo>
#include <QDir>
#include <QPixmap>

namespace mccmap {

TileLoader::TileLoader(FileCache* cache, QObject* parent)
    : QObject(parent)
    , _multi(this)
    , _mapInfo(cache)
    , _zoomLevel(0)
    , _downloadEnabled(true)
{
    connect(cache, &FileCache::cacheReloaded, this, &TileLoader::cacheReloaded);
    for (std::size_t i = 0; i < numDownloaders; i++) {
        auto& handle = _handles[i];
        handle.easy = _multi.addTransfer();

        handle.easy->setWriteFunction([&handle](const void* buf, std::size_t size) -> std::size_t {
            handle.imgBuf.write(buf, size);
            return size;
        });

        const char* userAgent = "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10; rv:33.0) Gecko/20100101 Firefox/33.0";
        handle.easy->set(CURLOPT_USERAGENT, userAgent);
        handle.easy->set(CURLOPT_NOSIGNAL, 1);
        handle.easy->set(CURLOPT_TIMEOUT, 5);
        handle.easy->set(CURLOPT_SSL_VERIFYPEER, 0L);

        connect(handle.easy, &CurlEasy::done, this, [this, &handle](CURLcode code) {
            if (code == CURLE_OK) {
                QPixmap pixmap;
                if (pixmap.loadFromData(handle.imgBuf.data(), handle.imgBuf.size())) {
                    emit pixmapReady(handle.pos, pixmap);
                    saveImg(handle.pos, handle.imgBuf);
                } else {
                    emit pixmapFailed(handle.pos);
                    BMCL_DEBUG() << "failed to load downloaded pixmap " << handle.url;
                }
            } else {
                emit pixmapFailed(handle.pos);
                BMCL_DEBUG() << "failed to download pixmap " << handle.url + " " + curl_easy_strerror(code);
            }
            assert(_numUsedHandles != 0);

            handle.imgBuf.resize(0);

            if (!_downloadQueue.empty()) {
                handle.pos = _downloadQueue.back();
                auto url = _mapInfo->generateTileUrl(handle.pos);
                _downloadQueue.pop_back();
                if (url.isNone()) {
                    _numUsedHandles--;
                    return;
                }

                handle.url = url.take();
                assert(!handle.easy->isRunning());
                handle.easy->set(CURLOPT_URL, handle.url.data());
                handle.easy->perform();
            } else {
                assert(_numUsedHandles != 0);
                _numUsedHandles--;
            }

        });
    }
    _numUsedHandles = 0;
}

TileLoader::~TileLoader()
{
    disconnect(_mapInfo.get(), &FileCache::cacheReloaded, this, 0);
}

void TileLoader::saveImg(const TilePosition& pos, const bmcl::Buffer& img)
{
    auto path = _mapInfo->generateTileSavePath(pos);
    if (path.isNone()) {
        return;
    }
    QDir().mkpath(QFileInfo(path.unwrap()).absolutePath());

    QFile file(path.unwrap());
    bool isOk = file.open(QIODevice::WriteOnly);
    if (!isOk) {
        BMCL_DEBUG() << "failed to open cache file for write: " << path->toStdString();
        return;
    }

    int64_t size = file.write((const char*)img.data(), img.size());
    if (size != img.size()) {
        BMCL_DEBUG() << "failed to write cache file: " << path->toStdString();
        return;
    }
}

void TileLoader::setZoomLevel(int zoom)
{
    _zoomLevel = zoom;
}

void TileLoader::addRequest(const TilePosition& pos)
{
    if (_zoomLevel != pos.zoomLevel) {
        return;
    }

    auto pair = _mapInfo->loadTile(pos);
    if (pair.second != FileCache::TileType::Empty) {
        QPixmap pixmap = QPixmap::fromImage(pair.first);
        if (pixmap.isNull()) {
            emit pixmapFailed(pos);
        } else {
            emit pixmapReady(pos, pixmap);
        }
    }

    if (pair.second == FileCache::TileType::Original) {
        return;
    }

    if (!_downloadEnabled) {
        return;
    }

    assert(_numUsedHandles <= numDownloaders);
    if (_numUsedHandles == numDownloaders) {
        _downloadQueue.push_back(pos);
        return;
    }

    auto it = std::find_if(_handles.begin(), _handles.end(), [](const EasyPosAndUrl& handle) {
        return !handle.easy->isRunning();
    });

    EasyPosAndUrl& handle = *it;

    handle.pos = pos;
    auto url = _mapInfo->generateTileUrl(handle.pos);
    if (url.isNone()) {
        return;
    }
    _numUsedHandles++;

    handle.imgBuf.resize(0);
    handle.url = url.take();
    assert(!handle.easy->isRunning());
    handle.easy->set(CURLOPT_URL, handle.url.data());
    handle.easy->perform();
}

void TileLoader::cancelRequest(const TilePosition& pos)
{
}

void TileLoader::clear()
{
    _downloadQueue.clear();
}

void TileLoader::setDownloadEnabled(bool flag)
{
    _downloadEnabled = flag;
}
}
