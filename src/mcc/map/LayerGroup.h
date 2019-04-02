#pragma once

#include "mcc/map/Layer.h"
#include "mcc/map/Fwd.h"
#include "mcc/map/Rc.h"
#include "mcc/ui/Fwd.h"

#include <bmcl/Option.h>

#include <vector>

namespace mccmap {

class KmlModel;

class MCC_MAP_DECLSPEC LayerGroup : public Layer {
    Q_OBJECT
public:
    LayerGroup(const MapRect* rect);
    ~LayerGroup() override;
    void draw(QPainter* p) const override;
    void drawTiled(QPainter* p) const;
    void mouseLeaveEvent() override;
    bool mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos) override;
    bool mousePressEvent(const QPoint& pos) override;
    bool mouseReleaseEvent(const QPoint& pos) override;
    bool viewportResizeEvent(const QSize& oldSize, const QSize& newSize) override;
    bool viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos) override;
    bool zoomEvent(const QPoint& pos, int fromZoom, int toZoom) override;
    void changeProjection(const mccgeo::MercatorProjection& from, const mccgeo::MercatorProjection& to) override;
    bool viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport) override;
    void createMenues(const QPoint& pos, bool isSubmenu, QMenu* dest) override;
    bool mouseDoubleClickEvent(const QPoint& pos) override;

    void appendLayer(Layer* layer);
    void insertLayer(Layer* layer, std::size_t pos);
    void removeAt(std::size_t pos);

    void saveSettings(mccui::Settings* settings);
    void loadSettings(const mccui::Settings* settings);

    inline const Layer* layerAt(std::size_t index) const;
    inline Layer* layerAt(std::size_t index);
    bmcl::OptionPtr<const Layer> activeLayer() const;
    bmcl::OptionPtr<Layer> activeLayer();
    bmcl::OptionPtr<const Layer> activatedLayer() const;
    bmcl::OptionPtr<Layer> activatedLayer();
    inline std::size_t size() const;

    void setVisibleAt(std::size_t index, bool isVisible);
    void setEditableAt(std::size_t index, bool isEditable);
    void setActiveLayerAt(std::size_t index);
    void deactivateLayers();
    bool isActiveLayer(std::size_t index) const;

    bmcl::Option<std::size_t> indexOf(const Layer* layer) const;
    bmcl::OptionPtr<const Layer> layerWithName(const QString& name) const;
    //inline const std::vector<QPushButton*>& buttons() const;

signals:
    void activeLayerChanged(const bmcl::Option<std::size_t>& index);
    void layerAdded(const Layer* layer, std::size_t index);
    void layerRemoved(const Layer* layer, std::size_t index);

protected:
    const std::vector<Rc<Layer>>& layers() const;
    const std::vector<Rc<Layer>>& layers();

private:
    template <typename F, typename... A>
    void visitLayers(F&& func, A&&... args);
    template <typename F, typename L, typename... A>
    bool visitFirstAvailable(F&& func, L&& layerFunc, A&&... args);
    template <typename F, typename L, typename... A>
    bool visitFirstAvailableAfterActivated(F&& func, L&& layerFunc, A&&... args);
    bmcl::Option<std::size_t> _activatedLayer;
    bmcl::Option<std::size_t> _activeLayer;
    std::vector<Rc<Layer>> _layers;
    bool _catchUpdates;
};

inline std::size_t LayerGroup::size() const
{
    return _layers.size();
}

inline const Layer* LayerGroup::layerAt(std::size_t index) const
{
    return _layers[index].get();
}

inline Layer* LayerGroup::layerAt(std::size_t index)
{
    return _layers[index].get();
}

inline const std::vector<Rc<Layer>>& LayerGroup::layers()
{
    return _layers;
}

inline const std::vector<Rc<Layer>>& LayerGroup::layers() const
{
    return _layers;
}
}
