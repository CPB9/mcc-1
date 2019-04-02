#pragma once

#include "mcc/Config.h"
#include "mcc/map/drawables/Point.h"
#include "mcc/map/drawables/MarkerBase.h"
#include "mcc/map/drawables/Label.h"
#include "mcc/map/Rc.h"
#include "mcc/ui/Fwd.h"

#include <QString>

class QPainter;

namespace mccmap {

class MCC_MAP_DECLSPEC Flag : public AbstractMarker<Flag> {
public:
    Flag(const QPointF& position, const QPixmap& active, const QPixmap& inactive, const QString& name,
         const MapRect* rect, const mccui::CoordinateSystemController* csController);
    Flag(const MapRect* rect, const mccui::CoordinateSystemController* csController);
    ~Flag();

    void draw(QPainter* p, const MapRect* rect) const;
    QRectF rect() const;
    void changeZoomLevel(int from, int to);
    void drawWithoutLabel(QPainter* p, const MapRect* rect) const;
    void drawLabel(QPainter* p, const MapRect* rect) const;

    void setPosition(const QPointF& position);
    inline const QPointF& position() const;

    inline const QPixmap& activePixmap() const;
    inline const QPixmap& inactivePixmap() const;
    inline const QPointF& activeOffset() const;

    void setLabelBackground(const QColor& color);
    void setLabelScale(double scale);
    void setActivePixmap(const QPixmap& p);
    void setInactivePixmap(const QPixmap& p);
    void setLabel(const QString& label);
    void setAlignment(Qt::Alignment alignment);
    void setActiveAlignment(Qt::Alignment alignment);
    void setInactiveAlignment(Qt::Alignment alignment);
    bool isActive() const;
    void setActive(bool isActive);
    const QString& name() const;
    void setName(const QString& name);
    inline double labelScale() const;
    void updateLabel();

private:
    LabelBase _label;
    MarkerBase _activeMarker;
    MarkerBase _inactiveMarker;
    Point _point;
    QString _name;
    Rc<const MapRect> _rect;
    Rc<const mccui::CoordinateSystemController> _csController;
    bool _isActive;
};

inline const QPointF& Flag::position() const
{
    return _point.position();
}

inline double Flag::labelScale() const
{
    return _label.labelScale();
}

inline const QPixmap& Flag::activePixmap() const
{
    return _activeMarker.pixmap();
}

inline const QPixmap& Flag::inactivePixmap() const
{
    return _inactiveMarker.pixmap();
}

inline void Flag::setAlignment(Qt::Alignment alignment)
{
    setActiveAlignment(alignment);
    setInactiveAlignment(alignment);
}

inline void Flag::setActiveAlignment(Qt::Alignment alignment)
{
    _activeMarker.setAlignment(alignment);
}

inline void Flag::setInactiveAlignment(Qt::Alignment alignment)
{
    _inactiveMarker.setAlignment(alignment);
}

inline const QPointF& Flag::activeOffset() const
{
    return _activeMarker.offset();
}
}
