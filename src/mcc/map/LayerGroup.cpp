#include "mcc/map/LayerGroup.h"


#include "mcc/ui/Settings.h"
#include "mcc/map/OnlineCache.h"
#include "mcc/map/MapRect.h"

#include <bmcl/Buffer.h>
#include <bmcl/MemReader.h>
#include <bmcl/Logging.h>

#include <QPainter>
#include <QMenu>

namespace mccmap {

LayerGroup::LayerGroup(const MapRect* rect)
    : Layer(rect)
    , _catchUpdates(true)
{
}

LayerGroup::~LayerGroup()
{
}

void LayerGroup::appendLayer(Layer* layer)
{
    insertLayer(layer, _layers.size());
}

void LayerGroup::insertLayer(Layer* l, std::size_t pos)
{
    Rc<Layer> layer(l);
    _layers.insert(_layers.begin() + pos, layer.get());
    if (_activeLayer.isNone()) {
        _activeLayer = std::size_t(0);
    } else {
        if (pos <= _activeLayer.unwrap()) {
            _activeLayer.unwrap()++;
        }
    }
    if (_activatedLayer.isSome()) {
        if (pos <= _activatedLayer.unwrap()) {
            _activatedLayer.unwrap()++;
        }
    }
    connect(layer.get(), &Layer::sceneUpdated, this, [this]() {
        emit sceneUpdated();
    });
    connect(layer.get(), &Layer::activated, this, [this, layer]() {
        if (!_catchUpdates) {
            return;
        }
        _activeLayer = indexOf(layer.get());
        emit activeLayerChanged(_activeLayer);
        emit sceneUpdated();
    });
    connect(layer.get(), &Layer::visibilityChanged, this, &LayerGroup::sceneUpdated);
    connect(layer.get(), &Layer::editabilityChanged, this, &LayerGroup::sceneUpdated);
    if (layer->isActive()) {
        setActiveLayerAt(indexOf(layer.get()).unwrap());
    }
    emit layerAdded(layer.get(), pos);
}

#define IF_NO_LAYER_RETURN(index, ...)                                                                                 \
    if (index >= _layers.size()) {                                                                                     \
        assert(false); \
        BMCL_WARNING() << "Layer index out of bounds " << index;                                                       \
        return __VA_ARGS__;                                                                                            \
    }

static void adjustRemovedIndex(std::size_t pos, bmcl::Option<std::size_t>* index)
{
    if (index->isNone()) {
        return;
    }
    if (pos == index->unwrap()) {
        index->clear();
    } else if (pos < index->unwrap()) {
        index->unwrap()--;
    }
}

void LayerGroup::removeAt(std::size_t pos)
{
    IF_NO_LAYER_RETURN(pos);

    Layer* layer = _layers[pos].get();
    disconnect(layer, 0, this, 0);
    _layers.erase(_layers.begin() + pos);
    adjustRemovedIndex(pos, &_activeLayer);
    adjustRemovedIndex(pos, &_activatedLayer);
    emit layerRemoved(layer, pos);
}

static const char* stateKey = "map/layersState";

void LayerGroup::saveSettings(mccui::Settings* settings)
{
    bmcl::Buffer data;
    auto writeLayerName = [&data](const Layer* layer) {
        std::uint64_t size = std::strlen(layer->name());
        data.writeUint32Le(size);
        data.write(layer->name(), size);
        data.writeUint8('\0');
    };

    if (_activeLayer.isSome()) {
        data.writeUint8(true);
        writeLayerName(_layers[_activeLayer.unwrap()].get());
    } else {
        data.writeUint8(false);
    }

    for (const Rc<Layer>& layer : _layers) {
        writeLayerName(layer.get());
        data.writeUint8(layer->isVisible());
        data.writeUint8(layer->isEditable());
    }
    QByteArray byteArray((char*)data.data(), (int)data.size());
    settings->tryWrite(stateKey, byteArray);
}

void LayerGroup::loadSettings(const mccui::Settings* settings)
{
    QByteArray byteArray = settings->read(stateKey).toByteArray();
    bmcl::MemReader reader(byteArray.data(), byteArray.size());
    if (reader.isEmpty()) {
        return;
    }

    auto readNameAndFindLayer = [&reader, this]() {
        if (reader.sizeLeft() < 4) {
            return _layers.end();
        }

        std::uint32_t size = reader.readUint32Le();
        if (reader.sizeLeft() < size + 1) {
            return _layers.end();
        }

        const char* name = (char*)reader.current();
        reader.skip(size + 1);

        return std::find_if(_layers.begin(), _layers.end(), [name](const Rc<Layer>& desc) {
            return std::strcmp(name, desc->name()) == 0;
        });
    };

    bool hasActiveLayer = reader.readUint8();

    if (hasActiveLayer) {
        auto it = readNameAndFindLayer();
        if (it == _layers.end()) {
            return;
        }
        (*it)->setActive(true);
        _activeLayer = indexOf(it->get());
    }

    while (!reader.isEmpty()) {
        auto it = readNameAndFindLayer();
        if (it == _layers.end()) {
            return;
        }

        if (reader.sizeLeft() < 2) {
            return;
        }
        (*it)->setVisible(reader.readUint8());
        (*it)->setEditable(reader.readUint8());
    }
}

void LayerGroup::draw(QPainter* p) const
{
    for (const Rc<Layer>& layer : _layers) {
        if (layer->isVisible()) {
            layer->draw(p);
        }
    }
}

void LayerGroup::drawTiled(QPainter* p) const
{
    int maxMapSize = mapRect()->maxMapSize();
    int width = mapRect()->size().width();
    int x = mapRect()->mapOffsetRaw().x();

    for (const Rc<Layer>& layer : _layers) {
        if (layer->isVisible()) {
            for (int i = 0; i < width; i += maxMapSize) {
                layer->draw(p);
                p->translate(maxMapSize, 0);
            }

            if ((x + width) > maxMapSize) {
                layer->draw(p);
            }

            p->resetTransform();
        }
    }
}

template <typename F, typename... A>
inline void LayerGroup::visitLayers(F&& func, A&&... args)
{
    for (const Rc<Layer>& layer : _layers) {
        (layer.get()->*func)(std::forward<A>(args)...);
    }
}

//FIXME: убрать повторное прохождение слоев (2-3 раза)

template <typename F, typename L, typename... A>
inline bool LayerGroup::visitFirstAvailable(F&& func, L&& layerFunc, A&&... args)
{
    if (_activeLayer.isSome()) {
        Layer* desc = _layers[_activeLayer.unwrap()].get();
        if ((desc->*layerFunc)() && (desc->*func)(std::forward<A>(args)...)) {
            _activatedLayer = _activeLayer.unwrap();
            return true;
        }
    }
    for (auto it = _layers.rbegin(); it < _layers.rend(); it++) {
        if(((*it).get()->*layerFunc)() && (it->get()->*func)(std::forward<A>(args)...)) {
            _activatedLayer = std::distance(it, _layers.rend()) - 1;
            return true;
        }
    }
    _activatedLayer = bmcl::None;
    return false;
}

template <typename F, typename L, typename... A>
inline bool LayerGroup::visitFirstAvailableAfterActivated(F&& func, L&& layerFunc, A&&... args)
{
    if (_activatedLayer.isSome()) {
        Layer* desc = _layers[_activatedLayer.unwrap()].get();
        if ((desc->*layerFunc)() && (desc->*func)(std::forward<A>(args)...)) {
            return true;
        }
    }
    return visitFirstAvailable(std::forward<F>(func), std::forward<L>(layerFunc), std::forward<A>(args)...);
}

void LayerGroup::changeProjection(const mccgeo::MercatorProjection& from, const mccgeo::MercatorProjection& to)
{
    visitLayers(&Layer::changeProjection, from, to);
}

void LayerGroup::createMenues(const QPoint& pos, bool isSubmenu, QMenu* dest)
{
    (void)isSubmenu;
    auto createMenusForEditableLayers = [&pos, dest](const Rc<Layer>& layer) {
        if (layer->isVisibleAndEditable()) {
            layer->createMenues(pos, true, dest);
        }
    };
    if (_activatedLayer.isSome()) {
        std::size_t index = _activatedLayer.unwrap();

        if (index > 0) {
            std::for_each(_layers.begin(), _layers.begin() + index, createMenusForEditableLayers);
        }
        std::for_each(_layers.begin() + index + 1, _layers.end(), createMenusForEditableLayers);
        Layer* activated = _layers[_activatedLayer.unwrap()].get();
        if (activated->isVisibleAndEditable()) {
            activated->createMenues(pos, false, dest);
        }
    } else {
        std::for_each(_layers.begin(), _layers.end(), createMenusForEditableLayers);
    }
}

bool LayerGroup::mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos)
{
    return visitFirstAvailableAfterActivated(&Layer::mouseMoveEvent, &Layer::isVisibleAndEditable, oldPos, newPos);
}

bool LayerGroup::mousePressEvent(const QPoint& pos)
{
    return visitFirstAvailableAfterActivated(&Layer::mousePressEvent, &Layer::isVisibleAndEditable, pos);
}

bool LayerGroup::mouseDoubleClickEvent(const QPoint& pos)
{
    return visitFirstAvailableAfterActivated(&Layer::mouseDoubleClickEvent, &Layer::isVisibleAndEditable, pos);
}

bool LayerGroup::mouseReleaseEvent(const QPoint& pos)
{
    //HACK
    return visitFirstAvailableAfterActivated(&Layer::mouseReleaseEvent, &Layer::isVisibleAndEditable, pos);
}

void LayerGroup::mouseLeaveEvent()
{
    visitLayers(&Layer::mouseLeaveEvent);
}

bool LayerGroup::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewpiort, const QRect& newViewport)
{
    visitLayers(&Layer::viewportResetEvent, oldZoom, newZoom, oldViewpiort, newViewport);
    return true;
}

bool LayerGroup::viewportResizeEvent(const QSize& oldSize, const QSize& newSize)
{
    visitLayers(&Layer::viewportResizeEvent, oldSize, newSize);
    return true;
}

bool LayerGroup::viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos)
{
    visitLayers(&Layer::viewportScrollEvent, oldPos, newPos);
    return true;
}

bool LayerGroup::zoomEvent(const QPoint& pos, int fromZoom, int toZoom)
{
    visitLayers(&Layer::zoomEvent, pos, fromZoom, toZoom);
    return true;
}

void LayerGroup::setEditableAt(std::size_t index, bool isEditable)
{
    IF_NO_LAYER_RETURN(index);
    if (_layers[index]->isEditable() == isEditable) {
        return;
    }

    _layers[index]->setEditable(isEditable);
}

void LayerGroup::setVisibleAt(std::size_t index, bool isVisible)
{
    IF_NO_LAYER_RETURN(index);
    if (_layers[index]->isVisible() == isVisible) {
        return;
    }

    _layers[index]->setVisible(isVisible);
}

bool LayerGroup::isActiveLayer(std::size_t index) const
{
    IF_NO_LAYER_RETURN(index, false);
    if (_activeLayer.isSome()) {
        return index == _activeLayer.unwrap();
    }
    return false;
}

bmcl::Option<std::size_t> LayerGroup::indexOf(const Layer* layer) const
{
    auto it = std::find_if(_layers.begin(), _layers.end(), [layer](const Rc<Layer>& desc) {
        return desc == layer;
    });
    if (it == _layers.end()) {
        return bmcl::None;
    }
    return bmcl::Option<std::size_t>(std::distance(_layers.begin(), it));
}

bmcl::OptionPtr<const Layer> LayerGroup::layerWithName(const QString& name) const
{
    auto it = std::find_if(_layers.begin(), _layers.end(), [&name](const Rc<Layer>& desc) {
        return QString::fromUtf8(desc->name()) == name;
    });
    if (it == _layers.end()) {
        return bmcl::None;
    }
    return _layers[std::distance(_layers.begin(), it)].get();
}

void LayerGroup::setActiveLayerAt(std::size_t index)
{
    IF_NO_LAYER_RETURN(index);

    if (_activeLayer.isSome()) {
        if (index == _activeLayer.unwrap()) {
            return;
        }
        _catchUpdates = false;
        _layers[_activeLayer.unwrap()]->setActive(false);
    }
    _catchUpdates = false;
    _activeLayer = index;
    _layers[index]->setActive(true);
    _catchUpdates = true;
    emit activeLayerChanged(index);
}

void LayerGroup::deactivateLayers()
{
    if (_activeLayer.isSome()) {
        _catchUpdates = false;
        _layers[_activeLayer.unwrap()]->setActive(false);
        _catchUpdates = true;
        _activeLayer = bmcl::None;
    }
}

bmcl::OptionPtr<const Layer> LayerGroup::activeLayer() const
{
    if (_activeLayer.isSome()) {
        return _layers[_activeLayer.unwrap()].get();
    }
    return bmcl::None;
}

bmcl::OptionPtr<Layer> LayerGroup::activeLayer()
{
    if (_activeLayer.isSome()) {
        return _layers[_activeLayer.unwrap()].get();
    }
    return bmcl::None;
}

bmcl::OptionPtr<const Layer> LayerGroup::activatedLayer() const
{
    if (_activatedLayer.isSome()) {
        return _layers[_activatedLayer.unwrap()].get();
    }
    return bmcl::None;
}

bmcl::OptionPtr<Layer> LayerGroup::activatedLayer()
{
    if (_activatedLayer.isSome()) {
        return _layers[_activatedLayer.unwrap()].get();
    }
    return bmcl::None;
}
}
