#include "BackgroundedTextItem.h"

#include <QPainter>
#include <QStaticText>
#include <QApplication>
#include <QFontMetricsF>

namespace mcc3d {

BackgroundedTextItem::BackgroundedTextItem(QGraphicsItem *parent) :
    QGraphicsRectItem(parent),
    _radius(8.0),
    _text(new QStaticText()),
    _font(nullptr),
    _backColor(0, 0, 0, 128)
{
    init();
}

BackgroundedTextItem::BackgroundedTextItem(const QString &text, QGraphicsItem *parent) :
    BackgroundedTextItem(parent)
{
    init();
    setText(text);
}

BackgroundedTextItem::~BackgroundedTextItem()
{
    delete _font;
    delete _text;
}

void BackgroundedTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setFont(*_font);
    painter->setRenderHint(QPainter::Antialiasing);

    painter->setBrush(_backColor);
    painter->setPen(QColor(0, 0, 0, 0));
    painter->drawRoundedRect(rect(), _radius, _radius);

    painter->setPen(_color);
    painter->drawStaticText(this->rect().topLeft() + QPointF(_radius * 0.625, (this->rect().height() - this->rect().height()/1.25)/2.0), *_text);
}

void BackgroundedTextItem::setText(const QString &text)
{
    _text->setText(text);
    updateRect();
}

void BackgroundedTextItem::setColor(const QColor &color)
{
    if(_color != color)
    {
        _color = color;
        update();
    }
}

void BackgroundedTextItem::setBackgroundColor(const QColor &backColor)
{
    if(_backColor != backColor)
    {
        _backColor = backColor;
        update();
    }
}

void BackgroundedTextItem::setFontSize(qreal pointSize)
{
    if(_font && pointSize > 0.0)
    {
        _font->setPointSizeF(pointSize);
        updateRect();
    }
}

void BackgroundedTextItem::setFont(const QFont &font)
{
    if(_font)
    {
        *_font = font;
        updateRect();
    }
}

QString BackgroundedTextItem::text() const
{
    return _text->text();
}

void BackgroundedTextItem::init()
{
    if(!_font)
    {
        _font = new QFont("Roboto Condensed");
        _font->setStyleName("Light");
        _font->setPointSizeF(_font->pointSizeF() * 2.0);
    }
}

void BackgroundedTextItem::updateRect()
{
    if(!_font)
        return;

    QFontMetricsF metrics(*_font);
    QRectF rect = metrics.boundingRect(_text->text());

    QRectF newRect;
    newRect.setWidth(rect.width());
    newRect.setHeight(rect.height() * 1.25);
    _radius = newRect.height() * 0.5;
    newRect.adjust(-_radius * 0.625, 0.0, _radius * 0.625, 0.0);
    setRect(newRect);
}

}
