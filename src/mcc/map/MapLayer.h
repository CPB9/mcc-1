#pragma once

#include "mcc/Config.h"
#include "mcc/map/Layer.h"
#include "mcc/map/ChannelConsumer.h"
#include "mcc/map/Channel.h"
#include "mcc/map/Fwd.h"
#include "mcc/map/TilePosition.h"
#include "mcc/map/MemoryCache.h"
#include "mcc/map/Rc.h"
#include "mcc/geo/Fwd.h"

#include <bmcl/Buffer.h>
#include <bmcl/Fwd.h>

#include <QObject>

#include <mutex>
#include <utility>

class QAction;
class QThread;

namespace mccmap {

class TileLoader;
class MercatorProjection;

class MCC_MAP_DECLSPEC MapLayer : public Layer {
    Q_OBJECT
public:
    MapLayer(const MapRect* rect, const mccui::CoordinateSystemController* csController);
    ~MapLayer() override;

    void draw(QPainter* p) const override;
    bool viewportResizeEvent(const QSize& oldSize, const QSize& newSize) override;
    bool viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos) override;
    bool zoomEvent(const QPoint& pos, int fromZoom, int toZoom) override;
    bool viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport) override;
    void changeProjection(const mccgeo::MercatorProjection& from, const mccgeo::MercatorProjection& to) override;
    const char* name() const override;
    void createMenues(const QPoint& pos, bool isSubmenu, QMenu* dest) override;

    void setMapInfo(FileCache* mapInfo, bool downloadEnabled);
    void enableTileDownloading(bool flag);

    void setPrintAction(QAction* act);

signals:
    void tileLoadRequested(const TilePosition& pos);

public slots:
    void reload();

private slots:
    void onResize(const QSize& size);
    void onScroll(int dx, int dy);
    void onRectChanged(int zoomLevel, const QSize& size, const QPoint& offset);

private:
    struct DownloadMsg {
        DownloadMsg(const TilePosition& pos, std::string&& url, bmcl::Option<QString>&& path)
            : pos(pos)
            , downloadUrl(std::move(url))
            , savePath(std::move(path))
        {
        }

        TilePosition pos;
        std::string downloadUrl;
        bmcl::Option<QString> savePath;
    };

    void connectManager();
    void copy(const mccgeo::LatLon& latLon);
    void clear();
    double relativeOffset(int y) const;
    void adjustMapOffsets();

    void sendTiles(const std::vector<TilePosition>& tiles);

    bool _downloadEnabled;
    bool _hasOnlineTiles;

    QPoint _paintOffset;

    MemoryCache _cache;
    TileLoader* _manager;
    QThread* _managerThread;

    QAction* _copyAction;
    QAction* _printAction;
    QPointF _menuPos;
    Rc<const mccui::CoordinateSystemController> _csController;
};
}
