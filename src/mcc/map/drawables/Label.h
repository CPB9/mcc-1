#pragma once

#include "mcc/Config.h"
#include "mcc/map/drawables/Interfaces.h"
#include "mcc/map/drawables/Point.h"

#include <QString>
#include <QColor>
#include <QPointF>
#include <QPixmap>

namespace mccmap {

class MCC_MAP_DECLSPEC LabelBase {
public:
    LabelBase(const QString& label = QString());
    ~LabelBase();

    void drawAt(QPainter* p, const QPointF& pos) const;
    static QImage createLabelImage(const QString& text, const QColor& color);
    static void drawLabelAt(QPainter* p, const QString& text, const QPointF& point, const QColor& color);
    static void drawLabelAt(QPainter* p, const QString& text, const QPointF& point, const QColor& color,
                            Qt::Alignment alignment, double scale = 1);
    static QSize computeSize(const QString& text);
    void setLabel(const QString& label);
    inline const QString& label() const;
    void setLabelBackground(const QColor& color);
    void setLabelScale(double scale);
    void setLabelParams(const QString& label, const QColor& color, double scale);
    void setLabelAlignment(Qt::Alignment alignment);
    inline double labelScale() const;

protected:
    inline const QPixmap& labelPixmap() const;
    void repaintLabelPixmap();

    QString _label;
    QColor _background;
    QPointF _offset;
    QPixmap _renderedLabel;
    double _labelScale;
    Qt::Alignment _alignment;
};

inline const QString& LabelBase::label() const
{
    return _label;
}

inline const QPixmap& LabelBase::labelPixmap() const
{
    return _renderedLabel;
}

inline double LabelBase::labelScale() const
{
    return _labelScale;
}

class MCC_MAP_DECLSPEC Label : public AbstractMarker<Label>, public LabelBase {
public:
    Label(const QPointF& position = QPointF(0, 0), const QString& label = QString());
    ~Label();

    void draw(QPainter* p, const MapRect* rect) const;
    inline QRectF rect() const;
    inline void changeZoomLevel(int from, int to);

    inline const QPointF& position() const;
    inline void setPosition(const QPointF& position);

private:
    Point _point;
};

inline void Label::changeZoomLevel(int from, int to)
{
    _point.changeZoomLevel(from, to);
}

inline QRectF Label::rect() const
{
    return WithRect::positionedRect(labelPixmap().rect(), _point.position());
}

inline const QPointF& Label::position() const
{
    return _point.position();
}

inline void Label::setPosition(const QPointF& position)
{
    _point.setPosition(position);
}
}
