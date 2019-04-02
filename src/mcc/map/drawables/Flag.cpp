#include "mcc/map/drawables/Flag.h"
#include "mcc/ui/CoordinateSystemController.h"

namespace mccmap {

Flag::Flag(const QPointF& position, const QPixmap& active, const QPixmap& inactive, const QString& name,
           const MapRect* rect, const mccui::CoordinateSystemController* csController)
    : _name(name)
    , _point(position)
    , _rect(rect)
    , _csController(csController)
{
    _isActive = false;
    _label.setLabel(name);
    _label.setLabelAlignment(Qt::AlignBottom | Qt::AlignLeft);
    _activeMarker.setPixmap(active);
    _activeMarker.setAlignment(Qt::AlignBottom);
    _inactiveMarker.setPixmap(inactive);
    _inactiveMarker.setAlignment(Qt::AlignBottom);
}

Flag::Flag(const MapRect* rect, const mccui::CoordinateSystemController* csController)
    : Flag(QPointF(0, 0), QPixmap(), QPixmap(), QString(), rect, csController)
{
}

Flag::~Flag()
{
}

void Flag::setActive(bool isActive)
{
    _isActive = isActive;
    updateLabel();
}

void Flag::setLabelBackground(const QColor& color)
{
    _label.setLabelBackground(color);
}

void Flag::setLabelScale(double scale)
{
    _label.setLabelScale(scale);
}

void Flag::updateLabel()
{
    if (_isActive) {
        mccgeo::LatLon latLon = toLatLon(_point.position(), _rect.get());
        _label.setLabel(_name + _csController->convertAndFormat(latLon, " (%1 %2)"));
    } else {
        _label.setLabel(_name);
    }
}

bool Flag::isActive() const
{
    return _isActive;
}

void Flag::changeZoomLevel(int from, int to)
{
    _point.changeZoomLevel(from, to);
}

void Flag::draw(QPainter* p, const MapRect* rect) const
{
    drawWithoutLabel(p, rect);
    drawLabel(p, rect);
}

void Flag::drawWithoutLabel(QPainter* p, const MapRect* rect) const
{
    if (_isActive) {
        _activeMarker.drawMarker(p, _point.position() - rect->mapOffsetRaw());
    } else {
        _inactiveMarker.drawMarker(p, _point.position() - rect->mapOffsetRaw());
    }
}

void Flag::drawLabel(QPainter* p, const MapRect* rect) const
{
    _label.drawAt(p, this->rect().topRight() - rect->mapOffsetRaw());
}

void Flag::setActivePixmap(const QPixmap& p)
{
    _activeMarker.setPixmap(p);
}

void Flag::setInactivePixmap(const QPixmap& p)
{
    _inactiveMarker.setPixmap(p);
}

void Flag::setLabel(const QString& label)
{
    _name = label;
    updateLabel();
}

QRectF Flag::rect() const
{
    if (isActive()) {
        return WithRect::positionedRect(_activeMarker.rect(), _point.position() + _activeMarker.offset());
    }
    return WithRect::positionedRect(_inactiveMarker.rect(), _point.position() + _inactiveMarker.offset());
}

void Flag::setName(const QString& name)
{
    _name = name;
    updateLabel();
}

const QString& Flag::name() const
{
    return _name;
}

void Flag::setPosition(const QPointF& position)
{
    _point.setPosition(position);
    updateLabel();
}
}
