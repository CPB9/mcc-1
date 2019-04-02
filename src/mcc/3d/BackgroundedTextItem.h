#pragma once

#include "mcc/Config.h"

#include <QGraphicsRectItem>

class QStaticText;

namespace mcc3d {

class MCC_3D_DECLSPEC BackgroundedTextItem : public QGraphicsRectItem
{
public:
    explicit BackgroundedTextItem(QGraphicsItem* parent = Q_NULLPTR);
    explicit BackgroundedTextItem(const QString& text, QGraphicsItem* parent = Q_NULLPTR);
    ~BackgroundedTextItem();

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = Q_NULLPTR);

    void setText(const QString& text);
    void setColor(const QColor& color);
    void setBackgroundColor(const QColor& backColor);
    void setFontSize(qreal pointSize);
    void setFont(const QFont& font);
    void fitInBox(const QSizeF& size);

    QString text() const;
    qreal radius() const {return _radius;}
    QColor color() const {return _color;}
    QColor backgroundColor() const {return _backColor;}

private:
    void init();
    void updateRect();

private:
    qreal        _radius;
    QStaticText* _text;
    QFont*       _font;
    QColor       _color;
    QColor       _backColor;

    Q_DISABLE_COPY(BackgroundedTextItem)
};
}
