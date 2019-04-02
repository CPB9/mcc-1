#pragma once

#include "mcc/Config.h"

#include "mcc/map/TilePosition.h"
#include "mcc/map/CurlMulti.h"
#include "mcc/map/Rc.h"

#include <bmcl/Buffer.h>

class QPixmap;

namespace mccmap {

class CurlEasy;
class FileCache;

class MCC_MAP_DECLSPEC TileLoader : public QObject {
    Q_OBJECT
public:
    static constexpr const int numDownloaders = 4;

    TileLoader(FileCache* cache, QObject* parent = nullptr);
    ~TileLoader();

signals:
    void pixmapReady(const TilePosition& pos, const QPixmap& pixmap);
    void pixmapFailed(const TilePosition& pos);
    void cacheReloaded();

public slots:
    void addRequest(const TilePosition& pos);
    void cancelRequest(const TilePosition& pos);
    void clear();
    void setZoomLevel(int zoom);
    void setDownloadEnabled(bool flag);

private:
    void saveImg(const TilePosition& pos, const bmcl::Buffer& img);

    struct EasyPosAndUrl {
        CurlEasy* easy;
        TilePosition pos;
        std::string url;
        bmcl::Buffer imgBuf;
    };

    std::array<EasyPosAndUrl, numDownloaders> _handles;
    std::size_t _numUsedHandles;
    std::vector<TilePosition> _downloadQueue;
    CurlMulti _multi;
    Rc<FileCache> _mapInfo;
    int _zoomLevel;
    bool _downloadEnabled;
};
}
