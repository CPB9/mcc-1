#include "mcc/map/MapLayer.h"
#include "mcc/map/MapRect.h"
#include "mcc/map/FileCache.h"
#include "mcc/geo/MercatorProjection.h"
#include "mcc/geo/Coordinate.h"
#include "mcc/map/drawables/WithPosition.h"
#include "mcc/map/TileLoader.h"
#include "mcc/geo/LatLon.h"

#include <bmcl/Buffer.h>
#include <bmcl/Option.h>
#include <bmcl/Logging.h>

#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QThread>
#include <QApplication>
#include <QClipboard>

#include <cmath>

Q_DECLARE_METATYPE(mccmap::TilePosition);

namespace mccmap {

using namespace std::placeholders;

constexpr const int tileSize = 256;

static QImage createEmptyImage()
{
    QImage image(tileSize, tileSize, QImage::Format_ARGB32_Premultiplied);
    QPainter p(&image);
    p.fillRect(0, 0, tileSize, tileSize, Qt::gray);
    return image;
}

MapLayer::MapLayer(const MapRect* rect, const mccui::CoordinateSystemController* csController)
    : Layer(rect)
    , _downloadEnabled(false)
    , _paintOffset(0, 0)
    , _csController(csController)
{
    qRegisterMetaType<TilePosition>();
    _managerThread = new QThread(this);
    _manager = new TileLoader(new EmptyFileCache);
    _hasOnlineTiles = false;
    _cache.setDefaultPixmap(QPixmap::fromImage(createEmptyImage()));

    _copyAction = new QAction("Копировать координаты", this);
    connect(_copyAction, &QAction::triggered, this, [this, rect]() {
        mccgeo::LatLon latLon = mccmap::WithPosition<>::toLatLon(_menuPos, rect);
        copy(latLon);
    });

    _printAction = new QAction("Печать", this);
    _printAction->setDisabled(true);

    adjustMapOffsets();
}

MapLayer::~MapLayer()
{
    _managerThread->quit();
    _managerThread->wait();
    delete _managerThread;
    delete _manager;
}

void MapLayer::setPrintAction(QAction* act)
{
    _printAction = act;
}

void MapLayer::copy(const mccgeo::LatLon& latLon)
{ // WGS
    QMimeData* mData = _csController->makeMimeData(latLon);
    QApplication::clipboard()->setMimeData(mData);
}

void MapLayer::createMenues(const QPoint& pos, bool isSubmenu, QMenu* dest) {
    _menuPos = pos;
    BMCL_UNUSED(pos);
    QMenu* menu = createSubmenu("Карта", isSubmenu, dest);
    menu->addAction(_copyAction);
    menu->addAction(_printAction);
};

bool MapLayer::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport)
{
    (void)oldZoom;
    (void)oldViewpiort;
    onRectChanged(newZoom, newViewport.size(), mapRect()->mapOffset());
    return true;
}

bool MapLayer::viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos)
{
    QPoint delta = newPos - oldPos;
    onScroll(delta.x(), delta.y());
    return true;
}

bool MapLayer::zoomEvent(const QPoint& pos, int fromZoom, int toZoom)
{
    (void)pos;
    (void)fromZoom;
    (void)toZoom;
    onRectChanged(mapRect()->zoomLevel(), mapRect()->size(), mapRect()->mapOffset());
    return true;
}

bool MapLayer::viewportResizeEvent(const QSize& oldSize, const QSize& newSize)
{
    (void)oldSize;
    onResize(newSize);
    return true;
}

void MapLayer::changeProjection(const mccgeo::MercatorProjection& from, const mccgeo::MercatorProjection& to)
{
    // FIXME
    (void)from;
    (void)to;
    onRectChanged(mapRect()->zoomLevel(), mapRect()->size(), mapRect()->mapOffset());
}

void MapLayer::sendTiles(const std::vector<TilePosition>& tiles)
{
    for (const TilePosition& pos : tiles) {
        emit tileLoadRequested(pos);
    }
}

void MapLayer::enableTileDownloading(bool flag)
{
    _downloadEnabled = flag && _hasOnlineTiles;
    _manager->setDownloadEnabled(_downloadEnabled);

    if (_downloadEnabled) {
        auto queue = _cache.reloadCache();
        sendTiles(queue);
    } else {
        _manager->clear();
    }
}

void MapLayer::connectManager()
{
    connect(_manager, &TileLoader::pixmapReady, this, [this](const TilePosition& pos, const QPixmap& pixmap) {
        if (pos.zoomLevel == _cache.zoomLevel()) {
            _cache.updatePixmap(pos, pixmap);
            emit sceneUpdated();
        }
    }, Qt::QueuedConnection);

    connect(_manager, &TileLoader::cacheReloaded, this, &MapLayer::reload, Qt::QueuedConnection);

    //connect(_manager, &TileLoader::pixmapFailed, this, [this](const TilePosition& pos) {
    //    if (pos.zoomLevel == _cache.zoomLevel()) {
    //        _cache.resetPixmap(pos);
    //        emit sceneUpdated();
    //    }
    //}, Qt::QueuedConnection);

    connect(this, &MapLayer::tileLoadRequested, _manager, &TileLoader::addRequest, Qt::QueuedConnection);
}

void MapLayer::setMapInfo(FileCache* mapInfo, bool downloadEnabled)
{
    _manager->clear();
    _managerThread->quit();
    _managerThread->wait();

    delete _managerThread;
    _managerThread = new QThread(this);

    delete _manager;
    _manager = new TileLoader(mapInfo);

    _manager->setZoomLevel(_cache.zoomLevel());

    connectManager();

    _hasOnlineTiles = mapInfo->hasOnlineTiles();
    _downloadEnabled = downloadEnabled && _hasOnlineTiles;
    _manager->setDownloadEnabled(_downloadEnabled);

    _manager->moveToThread(_managerThread);
    _managerThread->start();

    auto imgs = _cache.reloadCache();
    sendTiles(imgs);
}

void MapLayer::reload()
{
    _manager->clear();
    auto imgs = _cache.reloadCache();
    sendTiles(imgs);
}

void MapLayer::clear()
{
    _manager->clear();
}

void MapLayer::draw(QPainter* p) const
{
    // p->fillRect(0, 0, _width, _height, Qt::red); //debug
    if (mapRect()->isCenteredVertically()) {
        p->fillRect(0, 0, mapRect()->size().width(), -_paintOffset.y(), Qt::gray);
        p->fillRect(0, -_paintOffset.y() + mapRect()->maxMapSize(), mapRect()->size().width(),
                    mapRect()->size().height(), Qt::gray);
    }
    auto t = p->transform();
    p->translate(-_paintOffset);
    _cache.drawNonTiled(p);
    p->setTransform(t);
}

void MapLayer::onScroll(int dx, int dy)
{
    adjustMapOffsets();
    QPoint newGlobalOffset = QPoint(mapRect()->offset().x() / tileSize,
                                    mapRect()->offset().y() / tileSize); // qpoint / int не работает
    QPoint globalOffsetDelta = newGlobalOffset - _cache.globalOffset();
    //TODO: join x and y updates
    //TODO: refact
    switch (globalOffsetDelta.x()) {
    case -1: {
        auto imgs = _cache.scrollLeft();
        sendTiles(imgs);
        break;
    }
    case  0:
        break;
    case  1: {
        auto imgs = _cache.scrollRight();
        sendTiles(imgs);
        break;
    }
    default:
        auto imgs = _cache.setPosition(_cache.zoomLevel(), newGlobalOffset.x(), newGlobalOffset.y());
        sendTiles(imgs);
        return;
    }
    switch (globalOffsetDelta.y()) {
    case -1: {
        auto imgs = _cache.scrollUp();
        sendTiles(imgs);
        break;
    }
    case  0:
        break;
    case  1: {
        auto imgs = _cache.scrollDown();
        sendTiles(imgs);
        break;
    }
    default:
        auto imgs = _cache.setPosition(_cache.zoomLevel(), newGlobalOffset.x(), newGlobalOffset.y());
        sendTiles(imgs);
        return;
    }
}

void MapLayer::adjustMapOffsets()
{
    _paintOffset.rx() = mapRect()->offset().x() % tileSize;
    if (mapRect()->isCenteredVertically()) {
        _paintOffset.ry() = (mapRect()->maxMapSize() - mapRect()->size().height()) / 2;
    } else {
        _paintOffset.ry() = mapRect()->offset().y() % tileSize;
    }
}

void MapLayer::onRectChanged(int zoomLevel, const QSize& size, const QPoint& offset)
{
    _manager->setZoomLevel(zoomLevel);

    Q_UNUSED(size);
    adjustMapOffsets();
    int tx = offset.x() / tileSize;
    int ty = offset.y() / tileSize;
    if (tx == _cache.globalOffsetX() && ty == _cache.globalOffsetY()) {
        return;
    }
    clear();
    auto imgs = _cache.setPosition(zoomLevel, tx, ty);
    sendTiles(imgs);
}

void MapLayer::onResize(const QSize& size)
{
    Q_UNUSED(size);
    int tileCountX = mapRect()->size().width() / tileSize + 2;
    int tileCountY = mapRect()->size().height() / tileSize + 2;
    adjustMapOffsets();
    auto imgs = _cache.resize(tileCountX, tileCountY);
    sendTiles(imgs);
}

const char* MapLayer::name() const
{
    return "Карта";
}
}
