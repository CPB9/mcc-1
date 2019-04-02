#include "mcc/map/MapWidget.h"

#include "mcc/ui/Settings.h"
#include "mcc/map/CacheStackModel.h"
#include "mcc/map/MapWidgetAnimator.h"
#include "mcc/map/FollowAnimator.h"
#include "mcc/map/LayerGroup.h"
#include "mcc/map/MapLayer.h"
#include "mcc/map/LayerWidget.h"
#include "mcc/map/MapRect.h"
#include "mcc/map/OnlineCache.h"
#include "mcc/map/StackCache.h"
#include "mcc/map/WebMapProperties.h"
#include "mcc/map/MapWidgetPlugin.h"
#include "mcc/ui/Trackable.h"
#include "mcc/map/drawables/WithPosition.h"
#include "mcc/map/UserWidget.h"
#include "mcc/map/LayerPlugin.h"
#include "mcc/plugin/PluginCache.h"

#include <bmcl/Logging.h>

#include <QApplication>
#include <QActionGroup>
#include <QMenu>
#include <QWidgetAction>
#include <QPainter>
#include <QPaintEvent>
#include <QSize>
#include <QPrintPreviewDialog>
#include <QPrinter>

#ifdef MCC_USE_OPENGL
#include <QSurfaceFormat>
#endif

#include <array>
#include <cmath>
#include <vector>

MCC_INIT_QRESOURCES(map);

namespace mccmap {

constexpr const char* latKey = "map/latitude";
constexpr const char* lonKey = "map/longitude";
constexpr const char* mapModeKey = "map/mapMode";
constexpr const char* zoomLevelKey = "map/zoom";
constexpr const char* staticMapTypeKey = "map/staticMapType";

MapWidget::MapWidget(MapRect* mapRect,
                     const mccui::CoordinateSystemController* csController,
                     mccui::Settings* settings,
                     QWidget* parent)
    : MCC_MAP_WIDGET_BASE(parent)
    , _rect(mapRect)
    , _currentMousePos(0, 0)
    , _openingContext(false)
    , _isMovingViewport(false)
    , _isLeftMouseDown(false)
    , _wasOnline(false)
    , _isCursorOverWidget(false)
    , _positionLoaded(false)
    , _trackable(std::make_unique<mccui::Trackable>())
    , _mapMode(mccui::MapMode::Online)
    , _isTrackingActivated(false)
    , _isTrackingAllowed(false)
    , _userWidgets()
    , _csController(csController)
    , _settings(settings)
{
    _latWriter = settings->acquireUniqueWriter(latKey, 55.7558).unwrap();
    _lonWriter = settings->acquireUniqueWriter(lonKey, 37.6173).unwrap();
    _mapModeWriter = settings->acquireUniqueWriter(mapModeKey, static_cast<uint>(_mapMode)).unwrap();
    _zoomLevelWriter = settings->acquireUniqueWriter(zoomLevelKey, 5).unwrap();
    _staticMapTypeWriter = settings->acquireUniqueWriter(staticMapTypeKey).unwrap();

    _contextMenu = nullptr;
    mapRect->setParent(this);
#ifdef MCC_USE_OPENGL
    QSurfaceFormat format = this->format();
    format.setSamples(4);
    format.setStencilBufferSize(8);
    setFormat(format);
#endif
    //setAttribute(Qt::WA_NoSystemBackground);
    //setAttribute(Qt::WA_OpaquePaintEvent);

    _printAction = new QAction("Печать", this);
    connect(_printAction, &QAction::triggered, this, [this]() {
        QPrintPreviewDialog dialog(this);

        connect(&dialog, &QPrintPreviewDialog::paintRequested, this, [this](QPrinter* printer) {
            renderLayers(printer);
        });
        dialog.exec();
    });

    _mapLayer = new MapLayer(mapRect, _csController.get());
    _mapLayer->setPrintAction(_printAction);

    setMouseTracking(true);
    setMinimumSize(400, 300);
    _animator.reset(new MapWidgetAnimator(this));
    _followAnimator.reset(new FollowAnimator(this));

    createStack();

    _layers = new LayerGroup(mapRect);
    _layers->insertLayer(_mapLayer.get(), 0);

    connect(_layers.get(), &Layer::sceneUpdated, this, [this]() { updateMap(); });

    createMapWidgets();

#if MCC_USE_OPENGL
    connect(this, &MapWidget::mapNeedsUpdate, this, [this]() { update(); }, Qt::QueuedConnection);
#endif

    resize(size());
    adjustChildren();
}

void MapWidget::loadPlugins(const mccplugin::PluginCache* cache)
{
    for (const auto& plugin : cache->plugins()) {
        if (plugin->hasTypeId(MapWidgetPlugin::id)) {
            auto s = static_cast<MapWidgetPlugin*>(plugin.get());
            auto w = s->takeMapWidget().release();
            addUserWidget(w);
        }
        if (plugin->hasTypeId(LayerPlugin::id)) {
            auto layer = static_cast<LayerPlugin*>(plugin.get())->layer();
            if (layer.isSome()) {
                _layers->appendLayer(layer.unwrap());
            }
        }
    }
    _cacheStackModel->loadPlugins(cache);
    _cacheStackModel->setEnabled(true);
    updateStackMenues();

    _staticMapType = _staticMapTypeWriter->read().toString().toStdString();
    setOnlineCache(_staticMapType);

    resize(size());
    adjustChildren();
    _layers->loadSettings(_settings.get());

    if (_mapMode == mccui::MapMode::Stack) { //HACK
        updateStackMenues();
        StackCache* cache = _cacheStackModel->stack();
        onCacheChanged(cache, false);
        updateMap();
    }
}

void MapWidget::updateMap()
{
#if MCC_USE_OPENGL
    emit mapNeedsUpdate();
#else
    update();
#endif
}

void MapWidget::createStack()
{
    _cacheStackModel = new CacheStackModel(_settings.get());
    connect(_cacheStackModel, &CacheStackModel::stackChanged, this, [this]() {
        if (_mapMode != mccui::MapMode::Stack) {
            return;
        }
        updateStackMenues();
        StackCache* cache = _cacheStackModel->stack();
        onCacheChanged(cache, false);
        updateMap();
    });
    //     connect(_cacheStackView, &CacheStackView::clicked, this, [this]() {
    //         _wasOnline = _onlineButton->isChecked();
    //         _cacheStackView->setEnabled(true);
    //         _onlineButton->setChecked(false);
    //         const StackCachePtr& cache = _cacheStackModel->stack();
    //         onCacheChanged(cache);
    //         updateMap();
    //     });
}

void MapWidget::createMapWidgets()
{
    _mapsActions = new QActionGroup(this);

    connect(_layers.get(), &LayerGroup::activeLayerChanged, this,
            [this](const bmcl::Option<std::size_t>& index)
    {
        if (index.isSome()) {
            _activeLayerName = _layers->layerAt(index.unwrap())->name();
        } else {
            _activeLayerName.clear();
        }
        activeLayerChanged(_activeLayerName);
    });

    _layersMenu = new QMenu;
    auto layerWidgetAction = new QWidgetAction(_layersMenu);
    _layerWidget = new LayerWidget(_layers.get());
    layerWidgetAction->setDefaultWidget(_layerWidget);
    _layersMenu->addAction(layerWidgetAction);
    _onlineMenu = new QMenu("Офлайн");
    _offlineAction = _onlineMenu->addAction("Офлайн");
    _onlineAction = _onlineMenu->addAction("Онлайн");
    _stackAction = _onlineMenu->addAction("Стек карт");
    _onlineActions = new QActionGroup(this);
    for (QAction* action : std::array<QAction*, 3>{{_offlineAction, _onlineAction, _stackAction}}) {
        action->setCheckable(true);
        _onlineActions->addAction(action);
    }
    _offlineAction->setChecked(true);

    _staticMapType = _staticMapTypeWriter->read().toString().toStdString();

    connect(_onlineAction, &QAction::triggered, this, [this]() {
        onOnlineModeChanged(mccui::MapMode::Online);
    });

    connect(_offlineAction, &QAction::triggered, this, [this]() {
        onOnlineModeChanged(mccui::MapMode::Offline);
    });

    connect(_stackAction, &QAction::triggered, this, [this]() {
        onOnlineModeChanged(mccui::MapMode::Stack);
    });

    mccui::MapMode mode = static_cast<mccui::MapMode>(_mapModeWriter->read().toUInt());
    QAction* modeAction;
    switch(mode) {
    case mccui::MapMode::Online:
        _mapMode = mode;
        modeAction = _onlineAction;
        break;
    case mccui::MapMode::Offline:
        _mapMode = mode;
        modeAction = _offlineAction;
        break;
    case mccui::MapMode::Stack: {
        _mapMode = mode;
        modeAction = _stackAction;
        break;
    }
    }
    modeAction->trigger();

    updateStackMenues();
    setOnlineCache(_staticMapType);
}

void MapWidget::setOnlineCache(const std::string& fullName)
{
    //adjustChildren();
    _followAnimator->stop();
    OnlineCache* cache = _cacheStackModel->onlineMapByName(fullName);
    onCacheChanged(cache, true);
    _cacheStackModel->selectOnlineMap(fullName);
    _staticMapType = fullName;
    adjustChildren();
    updateMap();
    if (isTrackingActivated()) {
        centerOnTrackable();
    }
}

void MapWidget::updateStackMenues()
{
    delete _contextMenu;
    _contextMenu = new QMenu;
    _mapsMenu = _contextMenu->addMenu("Карта");
    std::map<QString, std::map<QString, std::string>> menues;
    const auto& onlineCaches = _cacheStackModel->caches();
    for (const Rc<OnlineCache>& cache : onlineCaches) {
        menues[cache->createProvider()][cache->createName()] = cache->fullName();
    }

    std::map<std::string, QAction*> typeToAction;

    for (const auto& serviceMaps : menues) {
        QMenu* serviceMenu = _mapsMenu->addMenu(serviceMaps.first);
        for (const auto& nameAndType : serviceMaps.second) {
            QAction* action = serviceMenu->addAction(nameAndType.first);
            action->setCheckable(true);
            _mapsActions->addAction(action);
            typeToAction.emplace(nameAndType.second, action);
        }
    }

    if (typeToAction.size() > 0) {
        auto it = typeToAction.find(_staticMapType);
        if (it == typeToAction.end()) {
            typeToAction.begin()->second->setChecked(true);
            _staticMapType = typeToAction.begin()->first;
        } else {
            typeToAction[_staticMapType]->setChecked(true);
        }
    }

    for (const auto& kv : typeToAction) {
        const std::string& type = kv.first;
        connect(kv.second, &QAction::triggered, [this, type]() {
            setOnlineCache(type);
        });
    }
    setOnlineCache(_staticMapType);
}

void MapWidget::follow(const mccgeo::LatLon& latLon)
{
    QPointF nextOffset = _rect->mapOffset(latLon);
    _followAnimator->updateAircraftPosition(nextOffset);
}
void MapWidget::setTrackable(std::unique_ptr<mccui::Trackable>&& trackable)
{
    bool wasActivated = isTrackingActivated();

    stopTracking();
    _isTrackingAllowed = false;
    _trackable->disconnect(this);

    _trackable = std::move(trackable);
    connect(_trackable.get(), &mccui::Trackable::trackingEnabled, this, [this](bool isEnabled)
    {
        _isTrackingAllowed = isEnabled;
        if(!isTrackingAllowed())
            stopTracking();
        emit trackingStateUpdated();
    });
    connect(_trackable.get(), &mccui::Trackable::trackingStopped, this, [this]()
    {
        stopTracking();
    });
    if (wasActivated)
        startTracking();
}

QRect MapWidget::deadZone() const
{
    return _rect->deadZone();
}

void MapWidget::onOnlineModeChanged(mccui::MapMode mode)
{
    _mapMode = mode;

    switch(mode) {
    case mccui::MapMode::Online: {
        //_cacheStackModel->setEnabled(false);
        _mapLayer->enableTileDownloading(true);
        OnlineCache* cache = _cacheStackModel->onlineMapByName(_staticMapType);
        onCacheChanged(cache, true);
        break;
    }
    case mccui::MapMode::Offline: {
        //_cacheStackModel->setEnabled(false);
        _mapLayer->enableTileDownloading(false);
        OnlineCache* cache = _cacheStackModel->onlineMapByName(_staticMapType);
        onCacheChanged(cache, false);
        break;
    }
    case mccui::MapMode::Stack: {
        //_cacheStackModel->setEnabled(true);
        _mapLayer->enableTileDownloading(false);
        StackCache* cache = _cacheStackModel->stack();
        onCacheChanged(cache, false);
        break;
    }
    default:
        return;
    }

    updateMap();
    adjustChildren();
    _mapModeWriter->write(static_cast<uint>(mode));
}

void MapWidget::onCacheChanged(FileCache* cache, bool downloadEnabled)
{
    mccgeo::MercatorProjection p1 = _rect->projection();
    _mapLayer->setMapInfo(cache, downloadEnabled);
    mccgeo::MercatorProjection p2 = cache->projection();
    _rect->setProjection(p2);
    _layers->changeProjection(p1, p2);
    _mapName = cache->name();

    emit mapCacheChanged(_mapName, _mapMode);
    emit _rect->viewChanged();
}

MapWidget::~MapWidget()
{
    _layers->saveSettings(_settings.get());
    _zoomLevelWriter->write(_rect->zoomLevel());
    mccgeo::LatLon center = _rect->centerLatLon();
    _latWriter->write(center.latitude());
    _lonWriter->write(center.longitude());
    _staticMapTypeWriter->write(QString::fromStdString(_staticMapType));
    for (mccmap::UserWidget* w : _userWidgets) {
        delete w;
    }
    delete _mapsMenu;
    delete _contextMenu;
    delete _cacheStackModel;
    delete _onlineMenu;
    delete _layerWidget;
    delete _layersMenu;
}

void MapWidget::renderLayers(QPaintDevice* dev)
{
    auto oldSize = size();
    QSize newSize(dev->width(), dev->height());
    resizeLayersNoEmit(oldSize, newSize);
    renderNoResize(dev);
    resizeLayersNoEmit(newSize, oldSize);
}

void MapWidget::renderNoResize(QPaintDevice* dev)
{

    QPainter p(dev);
    p.setRenderHint(QPainter::Antialiasing);

    _layers->drawTiled(&p);

}

void MapWidget::paintEvent(QPaintEvent* event)
{
    event->accept();
    renderNoResize(this);
}

void MapWidget::resizeLayersNoEmit(const QSize& oldSize, const QSize& newSize)
{
    int minZoom = minimumAllowedZoomLevel(newSize);
    int zoom = _rect->zoomLevel();
    if (minZoom > zoom) {
        _rect->setZoomLevel(minZoom);
        _layers->zoomEvent(_rect->visibleMapRect().center().toPoint(), zoom, minZoom);
    }
    _rect->resize(newSize.width(), newSize.height());

    // TODO: reset
    _layers->viewportResizeEvent(oldSize, newSize);
}

void MapWidget::resizeEvent(QResizeEvent* event)
{
    event->accept();

    resizeLayersNoEmit(event->oldSize(), event->size());
    MCC_MAP_WIDGET_BASE::resizeEvent(event);

    updateMap();
    adjustChildren();

    emit _rect->viewChanged();
    emit allowedZoomLevelsChanged(minimumAllowedZoomLevel(), maximumAllowedZoomLevel());
}

void MapWidget::adjustChildren()
{
    QRect deadZone;
    int dw = 100 + _subWidgetMargin * 2; // FIXME: temp for ruler
    int dh = _subWidgetMargin * 2;

    deadZone.setX(width() - dw);
    deadZone.setY(height() - dh);
    deadZone.setWidth(dw);
    deadZone.setHeight(dh);

    _rect->setDeadZone(deadZone);

    setMinimumSize(dw, dh);

    for(UserWidget *w : _userWidgets)
    {
        w->adjustPosition();
    }
}

void MapWidget::mousePressEvent(QMouseEvent* event)
{
    emit mousePressed(event->button());

    if (_isLeftMouseDown) {
        return;
    }
    mouseMoveEvent(event);
    _animator->stop();
    stopTracking();
    QPoint pos = event->pos();
    QPoint mapOffset = _rect->mapOffset(pos.x(), pos.y());
    _rect->setCursorPosition(mapOffset);
    setFocus(Qt::MouseFocusReason);
    Qt::MouseButton button = event->button();
    if (button == Qt::LeftButton) {
        _isLeftMouseDown = true;
        event->accept();
        if (button & Qt::MiddleButton) {
            return;
        }
        _currentMousePos = pos;

        if (!_layers->mousePressEvent(mapOffset)) {
            _isMovingViewport = true;
        }
    } else if (button == Qt::MiddleButton) {
        event->accept();
        if (button & Qt::LeftButton) {
            return;
        }
        _isMovingViewport = true;
    }
}

void MapWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!_isCursorOverWidget) {
        emit latLonChanged(bmcl::None);
    }
    QPoint pos = event->pos();
    QPoint mapOffset = _rect->mapOffset(pos.x(), pos.y());
    _rect->setCursorPosition(mapOffset);
    if (_isMovingViewport && _settings->read("map/animation", true).toBool()) {
        _animator->start();
    }
    if (event->button() == Qt::LeftButton) {
        _isLeftMouseDown = false;
        _isMovingViewport = false;
        _layers->mouseReleaseEvent(mapOffset);
        updateMap();
        emit _rect->viewChanged();
    } else if (event->button() == Qt::MiddleButton) {
        _isMovingViewport = false;
        emit _rect->viewChanged();
    }
    mouseMoveEvent(event);
}

void MapWidget::scroll(const QPoint& delta)
{
    QPoint mapOffset = _rect->mapOffsetRaw();
    if (_rect->cursorPosition().isSome()) {
        _rect->setCursorPosition(_rect->cursorPosition().unwrap() - delta);
    }
    QPoint newMapOffset = mapOffset + delta;
    _rect->scroll(delta.x(), delta.y());
    _layers->viewportScrollEvent(mapOffset, newMapOffset);
    emit _rect->viewChanged();
    updateMap();
}

void MapWidget::mouseMoveEvent(const QPoint& pos)
{
    setFocus(Qt::MouseFocusReason);
    QPoint delta = pos - _currentMousePos;
    QPoint mapOffset = _rect->mapOffsetRaw();
    _rect->setCursorPosition(mapOffset + pos);
    if (_isMovingViewport) {
        stopTracking();
        QPoint newMapOffset = mapOffset + delta;
        _rect->scroll(delta.x(), delta.y());
        _layers->viewportScrollEvent(mapOffset, newMapOffset);
    } else {
        QPointF off = _rect->mapOffset(pos.x(), pos.y());
        QPointF newOff = off;
        WithPosition<>::moveBy(delta, _rect.get(), &newOff);
        _layers->mouseMoveEvent(off.toPoint(), newOff.toPoint());
        emit latLonChanged(_rect->latLon(pos));
    }
    _currentMousePos = pos;
    updateMap();
}

void MapWidget::mouseMoveEvent(QMouseEvent* event)
{
    _rect->setModifiers(QApplication::keyboardModifiers());
    mouseMoveEvent(event->pos());
}

void MapWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (_layers->mouseDoubleClickEvent(_rect->mapOffset(event->pos().x(), event->pos().y()))) {
        return;
    }
    if (event->button() == Qt::LeftButton) {
        zoom(event->pos(), 1);
        emit _rect->viewChanged();
    } else if (event->button() == Qt::RightButton) {
        zoom(event->pos(), -1);
        emit _rect->viewChanged();
    }
    event->accept();
}

void MapWidget::keyPressEvent(QKeyEvent* event)
{
    emit keyPressed(event->key());

    event->accept();
    _rect->setModifiers(event->modifiers());
    updateMap();
    int angle = 0;
    if (event->key() == Qt::Key_Equal || event->key() == Qt::Key_Plus) {
        angle = 1;
    } else if (event->key() == Qt::Key_Minus) {
        angle = -1;
    } else {
        QWidget::keyPressEvent(event);
        return;
    }
    if (_isLeftMouseDown) {
        return;
    }
    setZoomLevel(angle + _rect->zoomLevel());
    QWidget::keyPressEvent(event);
    emit _rect->viewChanged();
}

void MapWidget::keyReleaseEvent(QKeyEvent* event)
{
    event->accept();
    _rect->setModifiers(event->modifiers());
    updateMap();
    QWidget::keyReleaseEvent(event);
}

int MapWidget::minimumAllowedZoomLevel() const
{
    return minimumAllowedZoomLevel(size());
}

int MapWidget::maximumAllowedZoomLevel() const
{
    return mccmap::WebMapProperties::maxZoom();
}

int MapWidget::zoomLevel() const
{
    return mapRect()->zoomLevel();
}

int MapWidget::minimumAllowedZoomLevel(const QSize& size) const
{
    double zoom = std::log2(double(size.height()) / WebMapProperties::tilePixelSize());
    return static_cast<int>(std::ceil(zoom));
}

void MapWidget::setZoomLevel(int zoomLevel)
{
    if (zoomLevel > WebMapProperties::maxZoom()) {
        zoomLevel = WebMapProperties::maxZoom();
    }
    if (zoomLevel < WebMapProperties::minZoom()) {
        zoomLevel = WebMapProperties::minZoom();
    }
    QPoint p(width() / 2, height() / 2);
    zoom(p, zoomLevel - _rect->zoomLevel());
}

void MapWidget::zoom(const QPoint& pos, int angle)
{
    int oldZoom = _rect->zoomLevel();
    int newZoom = angle + oldZoom;
    if (newZoom > WebMapProperties::maxZoom() || newZoom < minimumAllowedZoomLevel()) {
        return;
    }
    QPoint mapOffset = _rect->mapOffset(pos.x(), pos.y());
    _rect->setCursorPosition(_rect->mapOffsetRaw() + mapFromGlobal(QCursor::pos()));
    _rect->zoom(pos, angle);
    _layers->zoomEvent(mapOffset, oldZoom, newZoom);
    if (_isTrackingActivated) {
        centerOnTrackable();
    }
    updateMap();
    emit _rect->viewChanged();
    emit zoomLevelChanged(newZoom);
}

void MapWidget::centerOnTrackable()
{
    bmcl::Option<mccgeo::LatLon> pos = _trackable->position();
    if (pos.isSome()) {
        centerOnNoDisconnect(pos.unwrap().latitude(), pos.unwrap().longitude(), _rect->zoomLevel());
    }
}

//FIXME: проследить кто владеет UserWidget
void MapWidget::addUserWidget(UserWidget* widget)
{
    if(widget == nullptr)
        return;

    for(UserWidget *w : _userWidgets) {
        if(w == widget)
            return;
    }

    widget->setMapWidget(this);
    _userWidgets.push_back(widget);
}

void MapWidget::startTracking()
{
    if(isTrackingActivated() || !isTrackingAllowed())
        return;

    connect(_trackable.get(), &mccui::Trackable::positionUpdated, this, [this](const mccgeo::LatLon& position) {
        follow(position);
    });
    centerOnTrackable();

    _isTrackingActivated = true;
    emit trackingStateUpdated();
}

void MapWidget::stopTracking()
{
    if(!isTrackingActivated())
        return;

    disconnect(_trackable.get(), &mccui::Trackable::positionUpdated, this, nullptr);

    _followAnimator->stop();
    _isTrackingActivated = false;
    emit trackingStateUpdated();
}

void MapWidget::setMapMode(mccui::MapMode mapMode)
{
    if(this->mapMode() == mapMode)
        return;

    onOnlineModeChanged(mapMode);
}

void MapWidget::wheelEvent(QWheelEvent* event)
{
    if (_isLeftMouseDown) {
        return;
    }
    setFocus(Qt::MouseFocusReason);
    int angle = event->angleDelta().y();
    if (angle == 0) {
        return;
    }
    angle = angle / std::abs(angle);
    zoom(event->pos(), angle);
}

void MapWidget::contextMenuEvent(QContextMenuEvent* event)
{
    if (_isLeftMouseDown) {
        return;
    }
    _openingContext = true;
    event->accept();
    QPoint pos = event->pos();
    mouseMoveEvent(pos);
    _animator->stop();
    stopTracking();
    QPoint mapOffset = _rect->mapOffset(pos.x(), pos.y());
    QMenu menu;
    _layers->createMenues(mapOffset, false, &menu);
    menu.exec(event->globalPos());
    QPoint newPos = mapFromGlobal(QCursor::pos());
    _layers->mouseMoveEvent(mapOffset, _rect->mapOffset(newPos.x(), newPos.y()));
}

void MapWidget::centerOn(double lat, double lon)
{
    centerOn(lat, lon, _rect->zoomLevel());
}

void MapWidget::centerOnNoDisconnect(double lat, double lon, int zoomLevel)
{
    _animator->stop();
    _followAnimator->stop();
    if (zoomLevel < minimumAllowedZoomLevel()) {
        zoomLevel = minimumAllowedZoomLevel();
    }
    if (zoomLevel > WebMapProperties::maxZoom()) {
        zoomLevel = WebMapProperties::maxZoom();
    }
    int oldZoom = _rect->zoomLevel();
    QRect oldViewport = _rect->visibleMapRect().toRect();
    _rect->centerOn(lat, lon, zoomLevel);
    int newZoom = _rect->zoomLevel();
    QRect newViewport = _rect->visibleMapRect().toRect();
    _layers->viewportResetEvent(oldZoom, newZoom, oldViewport, newViewport);
    updateMap();
    emit _rect->viewChanged();
    emit zoomLevelChanged(newZoom);
}

void MapWidget::centerOn(double lat, double lon, int zoomLevel)
{
    stopTracking();
    centerOnNoDisconnect(lat, lon ,zoomLevel);
}

void MapWidget::centerOn(const mccgeo::Bbox& bbox)
{
    _animator->stop();
    stopTracking();
    int oldZoom = _rect->zoomLevel();
    QRect oldViewport = _rect->visibleMapRect().toRect();
    _rect->centerOn(bbox);
    int newZoom = _rect->zoomLevel();
    QRect newViewport = _rect->visibleMapRect().toRect();
    _layers->viewportResetEvent(oldZoom, newZoom, oldViewport, newViewport);
    updateMap();
    emit zoomLevelChanged(newZoom);
}

void MapWidget::centerOn(const mccgeo::LatLon& latLon)
{
    centerOn(latLon.latitude(), latLon.longitude());
}

void MapWidget::enterEvent(QEvent* event)
{
    _isCursorOverWidget = true;
    QWidget::enterEvent(event);
}

void MapWidget::leaveEvent(QEvent* event)
{
    _isLeftMouseDown = false;
    _isCursorOverWidget = false;
    _rect->setCursorPosition(bmcl::None);
    if (!_openingContext) {
        _layers->mouseLeaveEvent();
        _openingContext = false;
    }
    _rect->setModifiers(QApplication::keyboardModifiers());
    emit latLonChanged(bmcl::None);
    QWidget::leaveEvent(event);
}

void MapWidget::showEvent(QShowEvent* event)
{
    if (_positionLoaded) {
        return;
    }
    double lat = _latWriter->read().toDouble();
    double lon = _lonWriter->read().toDouble();
    int zoom = _zoomLevelWriter->read().toInt();
    centerOn(lat, lon, zoom);
    _positionLoaded = true;
}
}
