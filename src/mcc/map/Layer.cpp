#include "mcc/map/Layer.h"
#include "mcc/map/drawables/WithPosition.h"
#include "mcc/map/drawables/Label.h"
#include "mcc/ui/CoordinateSystemController.h"
#include "mcc/ui/CoordinateSystemController.h"

#include <QPointF>
#include <QWidget>
#include <QWidgetAction>
#include <QMenu>
#include <QAction>
#include <QLabel>

namespace mccmap {

Layer::Layer(const MapRect* rect)
    : _isActive(false)
    , _isVisible(true)
    , _isEditable(true)
    , _rect(rect)
{
}

Layer::~Layer()
{
}

void Layer::drawCoordinatesAt(QPainter* p, const mccui::CoordinateSystemController* csController, const QPointF& coord, const QPointF& point, const QString& arg, double scale) const
{
    mccgeo::LatLon latLon = WithPosition<>::toLatLon(coord, mapRect());
    QString str = csController->convertAndFormat(latLon, arg);
    Label::drawLabelAt(p, str, point - _rect->mapOffsetRaw(), Qt::white, Qt::AlignBottom | Qt::AlignLeft, scale);
}

void Layer::setCursor(const QCursor& cursor)
{
    _rect->parent()->setCursor(cursor);
}

void Layer::createMenues(const QPoint& pos, bool isSubmenu, QMenu* dest)
{
    (void)isSubmenu;
    (void)pos;
    (void)dest;
}

QMenu* Layer::createSubmenu(const QString& name, bool isSubmenu, QMenu* dest)
{
    if (isSubmenu) {
        QMenu* menu = new QMenu;
        menu->setTitle(name);
        dest->addMenu(menu);
        return menu;
    }
    auto actWid = new QWidgetAction(dest);
    QLabel* header = new QLabel(name);
    header->setObjectName("contextMenuTitle");
    actWid->setDefaultWidget(header);
    dest->addAction(actWid);
    dest->setStyleSheet("QLabel#contextMenuTitle { background-color: blue; color: white; }");
    return dest;
}

const char* Layer::name() const
{
    return "NO NAME";
}

void Layer::setActive(bool isActive)
{
    _isActive = isActive;
    emit activated(isActive);
}

void Layer::setVisible(bool isVisible)
{
    _isVisible = isVisible;
    emit visibilityChanged(isVisible);
}

void Layer::setEditable(bool isEditable)
{
    _isEditable = isEditable;
    emit editabilityChanged(isEditable);
}

void Layer::mouseLeaveEvent()
{
}

bool Layer::mousePressEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool Layer::mouseReleaseEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool Layer::mouseMoveEvent(const QPoint& from, const QPoint& to)
{
    (void)from;
    (void)to;
    return false;
}

bool Layer::mouseDoubleClickEvent(const QPoint& pos)
{
    (void)pos;
    return false;
}

bool Layer::viewportResizeEvent(const QSize& oldSize, const QSize& newSize)
{
    (void)oldSize;
    (void)newSize;
    return false;
}

bool Layer::viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos)
{
    (void)oldPos;
    (void)newPos;
    return false;
}

bool Layer::viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewport, const QRect& newViewport)
{
    bool rv = false;
    QPoint center = mapRect()->centerMapOffset();
    rv |= zoomEvent(center, oldZoom, newZoom);
    rv |= viewportResizeEvent(oldViewport.size(), newViewport.size());
    rv |= viewportScrollEvent(oldViewport.topLeft(), newViewport.topLeft());
    return rv;
}
}
