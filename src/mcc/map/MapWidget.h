#pragma once

#include "mcc/Config.h"
#include "mcc/map/Fwd.h"
#include "mcc/map/Rc.h"
#include "mcc/geo/Fwd.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/MapMode.h"
#include "mcc/plugin/Fwd.h"

#include <bmcl/Fwd.h>

#ifdef MCC_USE_OPENGL
#define MCC_MAP_WIDGET_BASE QOpenGLWidget
#include <QOpenGLWidget>
#else
#define MCC_MAP_WIDGET_BASE QWidget
#include <QWidget>
#endif

#include <vector>

Q_DECLARE_METATYPE(mccui::MapMode);

class QMenu;
class QPaintDevice;
class QActionGroup;
class QAction;
class QLineEdit;
class QToolBar;

namespace mccmap {

class Layer;
class LayerWidget;
class MapLayer;
struct LayerDesc;
class KmlModel;
class CacheStackModel;
class MapWidgetAnimator;
class FollowAnimator;
class UserWidget;

class MCC_MAP_DECLSPEC MapWidget : public MCC_MAP_WIDGET_BASE {
    Q_OBJECT
public:
    MapWidget(MapRect* mapRect,
              const mccui::CoordinateSystemController* csController,
              mccui::Settings* settings,
              QWidget* parent = nullptr);
    ~MapWidget() override;

    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent*) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;

    void mouseMoveEvent(const QPoint& pos);

    void scroll(const QPoint& delta);
    void setTrackable(std::unique_ptr<mccui::Trackable>&& trackable);

    const MapRect* mapRect() const;
    inline CacheStackModel* cacheStackModel();

    static int subWidgetMargin() {return _subWidgetMargin;}
    QRect deadZone() const;

    void loadPlugins(const mccplugin::PluginCache* cache);

    void renderLayers(QPaintDevice* dev);
    int minimumAllowedZoomLevel() const;
    int maximumAllowedZoomLevel() const;
    int zoomLevel() const;

    QString mapName() const {return _mapName;}
    mccui::MapMode mapMode() const {return _mapMode;}
    QString activeLayerName() const {return _activeLayerName;}
    bool isTrackingActivated() const {return _isTrackingActivated;}
    bool isTrackingAllowed() const {return _isTrackingAllowed;}
    QMenu* mapsMenu() const {return _mapsMenu;}
    QMenu* layersMenu() const {return _layersMenu;}
    LayerGroup* layers() const {return _layers.get();}

signals:
    void mapNeedsUpdate();
    void latLonChanged(const bmcl::Option<mccgeo::LatLon>& latLon);
    void mousePressed(int button);
    void keyPressed(int key);
    void zoomLevelChanged(int level);
    void allowedZoomLevelsChanged(int minimum, int maximum);
    void mapCacheChanged(const QString& name, mccui::MapMode mapMode);
    void activeLayerChanged(const QString& name);
    void trackingStateUpdated();

public slots:
    void setZoomLevel(int zoomLevel);
    void zoom(const QPoint& pos, int angle);
    void centerOn(const mccgeo::LatLon& latLon);
    void centerOn(double lat, double lon);
    void centerOn(double lat, double lon, int zoomLevel);
    void centerOn(const mccgeo::Bbox& bbox);
    void centerOnTrackable();
    void addUserWidget(UserWidget *widget);
    void startTracking();
    void stopTracking();
    void setMapMode(mccui::MapMode mapMode);

private slots:
    void updateMap();

private:
    void resizeLayersNoEmit(const QSize& oldSize, const QSize& newSize);
    void renderNoResize(QPaintDevice* dev);
    void setOnlineCache(const std::string& name);
    void centerOnNoDisconnect(double lat, double lon, int zoomLevel);
    void follow(const mccgeo::LatLon& latLon);
    void createMapWidgets();
    void createStack();
    void updateStackMenues();
    void onCacheChanged(FileCache* cache, bool downloadEnabled);
    void onOnlineModeChanged(mccui::MapMode mode);
    void adjustChildren();
    int minimumAllowedZoomLevel(const QSize& size) const;

    static constexpr int _subWidgetMargin = 10;

    Rc<MapRect>                         _rect;
    Rc<LayerGroup>                      _layers;

    CacheStackModel*                    _cacheStackModel;

    QActionGroup*                       _mapsActions;
    QAction*                            _resetRulerAction;
    QAction*                            _printAction;

    QMenu*                              _contextMenu;
    QMenu*                              _mapsMenu;
    QMenu*                              _layersMenu;

    LayerWidget*                        _layerWidget;

    QAction*                            _onlineAction;
    QAction*                            _offlineAction;
    QAction*                            _stackAction;
    QActionGroup*                       _onlineActions;
    QMenu*                              _onlineMenu;

    std::unique_ptr<MapWidgetAnimator>  _animator;
    std::unique_ptr<FollowAnimator>     _followAnimator;

    QPoint                              _currentMousePos;

    bool                                _openingContext;
    bool                                _isMovingViewport;
    bool                                _isLeftMouseDown;
    bool                                _wasOnline;
    bool                                _isCursorOverWidget;
    bool                                _positionLoaded;
    std::string                         _staticMapType;
    std::unique_ptr<mccui::Trackable>   _trackable;
    Rc<MapLayer>                        _mapLayer;

    QString                             _mapName;
    mccui::MapMode                      _mapMode;
    QString                             _activeLayerName;
    bool                                _isTrackingActivated;
    bool                                _isTrackingAllowed;

    std::vector<UserWidget *>           _userWidgets;
    Rc<const mccui::CoordinateSystemController> _csController;
    Rc<mccui::Settings>                 _settings;
    Rc<mccui::SettingsWriter>           _latWriter;
    Rc<mccui::SettingsWriter>           _lonWriter;
    Rc<mccui::SettingsWriter>           _mapModeWriter;
    Rc<mccui::SettingsWriter>           _zoomLevelWriter;
    Rc<mccui::SettingsWriter>           _staticMapTypeWriter;
};

inline const MapRect* MapWidget::mapRect() const
{
    return _rect.get();
}

inline CacheStackModel* MapWidget::cacheStackModel()
{
    return _cacheStackModel;
}
}
