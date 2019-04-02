#pragma once

#include "mcc/Config.h"
#include "mcc/map/Fwd.h"
#include "mcc/map/Rc.h"
#include "mcc/geo/Fwd.h"
#include "mcc/ui/Fwd.h"
#include "mcc/ui/QObjectRefCountable.h"

#include <QObject>

class QPainter;
class QPoint;
class QPointF;
class QSize;
class QString;
class QCursor;
class QMenu;

namespace mccmap {

class MCC_MAP_DECLSPEC Layer : public mccui::QObjectRefCountable<QObject> {
    Q_OBJECT
public:
    Layer(const MapRect* rect);
    virtual ~Layer();

    virtual void draw(QPainter* p) const = 0;
    virtual void mouseLeaveEvent();
    virtual bool mousePressEvent(const QPoint& pos);
    virtual bool mouseReleaseEvent(const QPoint& pos);
    virtual bool mouseMoveEvent(const QPoint& oldPos, const QPoint& newPos);
    virtual bool mouseDoubleClickEvent(const QPoint& pos);
    virtual bool viewportResizeEvent(const QSize& oldSize, const QSize& newSize);
    virtual bool viewportScrollEvent(const QPoint& oldPos, const QPoint& newPos);
    virtual bool zoomEvent(const QPoint& pos, int fromZoom, int toZoom) = 0;
    virtual bool viewportResetEvent(int oldZoom, int newZoom, const QRect& oldViewport, const QRect& newViewport);
    virtual void createMenues(const QPoint& pos, bool isSubmenu, QMenu* dest);
    virtual const char* name() const;

    void drawCoordinatesAt(QPainter* p, const mccui::CoordinateSystemController* csController, const QPointF& coord,
                           const QPointF& point, const QString& arg = "%1, %2", double scale = 1) const;
    inline const MapRect* mapRect() const;
    inline bool isActive() const;
    inline bool isVisible() const;
    inline bool isEditable() const;
    inline bool isVisibleAndEditable() const;

public slots:
    virtual void changeProjection(const mccgeo::MercatorProjection& from, const mccgeo::MercatorProjection& to) = 0;
    virtual void setActive(bool isActive);
    virtual void setVisible(bool isVisible);
    virtual void setEditable(bool isEditable);

signals:
    void activated(bool isActive);
    void visibilityChanged(bool isVisible);
    void editabilityChanged(bool isEditable);
    void sceneUpdated();

protected:
    QMenu* createSubmenu(const QString& name, bool isSubmenu, QMenu* dest);
    void setCursor(const QCursor& cursor);

    bool _isActive;
    bool _isVisible;
    bool _isEditable;

private:
    Rc<const MapRect> _rect;
};

inline const MapRect* Layer::mapRect() const
{
    return _rect.get();
}

inline bool Layer::isActive() const
{
    return _isActive;
}

inline bool Layer::isVisible() const
{
    return _isVisible;
}

inline bool Layer::isEditable() const
{
    return _isEditable;
}

inline bool Layer::isVisibleAndEditable() const
{
    return _isVisible && _isEditable;
}
}
