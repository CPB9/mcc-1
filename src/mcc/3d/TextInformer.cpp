#include "TextInformer.h"

#include <QPainter>
#include <QApplication>
#include <QFontMetricsF>
#include <QFont>
#include <QSize>

#include <cassert>

#include <VasnecovUniverse>
#include <VasnecovFigure>
#include <VasnecovLabel>

namespace mcc3d
{
TextInformer::TextInformer(VasnecovUniverse* u, VasnecovWorld* w, const QSize& labelSize)
    : SimplestModel(u, w)
    , _label(nullptr)
    , _color(QColor(255, 255, 255, 255))
    , _isVisible(true)
    , _valueText()
    , _isRounded(true)
{
    assert(u != nullptr);
    assert(w != nullptr);

    _label = universe()->addLabel("Numerical label", world(), labelSize.width(), labelSize.height());
    assert(_label != nullptr);
}

TextInformer::~TextInformer()
{
    assert(universe() != nullptr);
    assert(world() != nullptr);

    universe()->removeLabel(_label);
}

void TextInformer::hide()
{
    setVisible(false);
}

void TextInformer::show()
{
    setVisible(true);
}

void TextInformer::setVisible(bool visible)
{
    if(visible != isVisible())
    {
        _isVisible = visible;

        if(_label)
            _label->setVisible(visible);
    }
}
void TextInformer::setColor(const QColor& c)
{
    if(c != color())
    {
        _color = c;
        redrawText();
    }
}

void TextInformer::setCoordinates(const QVector3D& coordinates)
{
    if(_label == nullptr)
        return;
    _label->setCoordinates(coordinates);
}

void TextInformer::attachToElement(const VasnecovAbstractElement* element)
{
    if(_label == nullptr)
        return;
    _label->attachToElement(element);
}

QVector3D TextInformer::coordinates() const
{
    if(_label == nullptr)
        return QVector3D();
    return _label->coordinates();
}

void TextInformer::setRounded(bool rounded)
{
    if(rounded == _isRounded)
        return;
    _isRounded = rounded;
    redrawText();
}

void TextInformer::setText(const QString& text)
{
    if(text == _valueText)
        return;

    _valueText = text;
    generateText();
}

void TextInformer::setOffset(const QPointF& offset)
{
    _label->setOffset(offset);
}

void TextInformer::setOffset(float x, float y)
{
    _label->setOffset(x, y);
}

QPointF TextInformer::offset() const
{
    return _label->offset().toPointF();
}

void TextInformer::generateText()
{
    redrawText();
}

void TextInformer::redrawText()
{
    drawText();
}
void TextInformer::drawText(bool bold, int size)
{
    if(_label == nullptr || _valueText.isEmpty())
        return;

    QSize imgSize = _label->size().toSize();
    QImage image(imgSize.width(), imgSize.height(), QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(0, 0, 0, 0));

    QPainter painter(&image);

    QFont font("Roboto Condensed");
    if(bold)
        font.setStyleName("Bold");
    if(size == 0)
        font.setPointSizeF(QApplication::font().pointSizeF() * 0.8);
    else
        font.setPixelSize(size);
    painter.setFont(font);

    QFontMetricsF metrics(font);
    QRectF rect = metrics.boundingRect(QRect(QPoint(), imgSize), 0, _valueText);

    QRectF newRect;
    newRect.setWidth(rect.width());
    newRect.setHeight(rect.height());
    newRect.adjust(0.0, 0.0, 8.0, 4.0);
    newRect.moveCenter(QPointF(imgSize.width() / 2, imgSize.height() / 2));

    painter.setBrush(QColor(0, 0, 0, 190));
    painter.setPen(QColor(0, 0, 0, 0));
    double r(_isRounded ? 4.0 : 0.0);
    painter.drawRoundedRect(newRect, r, r);

    painter.setPen(color());
    painter.drawText(QRectF(0, 0, imgSize.width(), imgSize.height()), Qt::AlignHCenter | Qt::AlignVCenter, _valueText);

    _label->setImage(image);
}

NumericalInformer::NumericalInformer(VasnecovUniverse* u, VasnecovWorld* w, const QString& suffix)
    : TextInformer (u, w, QSize(64, 32))
    , _suffix(suffix)
    , _value(0.0f)
{
}

NumericalInformer::~NumericalInformer()
{}

void NumericalInformer::setValue(float value)
{
    if(qFuzzyCompare(value, _value))
        return;

    _value = value;

    generateText();
}

void NumericalInformer::setPrecision(int precision)
{
    if(_precision == precision)
        return;

    _precision = precision;
    generateText();
}

void NumericalInformer::generateText()
{
    QString text = QString::number(_value, 'f', _precision) + _suffix;
    if(text == _valueText)
        return;

    _valueText = text;
    redrawText();
}



DistanceInformer::DistanceInformer(VasnecovUniverse* u, VasnecovWorld* w,
                                   const QString& postfix)
    : NumericalInformer (u, w, " m")
    , _line(nullptr)
    , _postfix(postfix)
{
    _isVisible = false;

    if (universe() != nullptr && world() != nullptr)
    {
        _line = universe()->addFigure("Range line", world());
        if (_line)
        {
            _line->setThickness(2.0f);
            _line->setColor(_color);

            _line->setVisible(_isVisible);
        }
        if (_label)
        {
            _label->setVisible(_isVisible);
        }
    }
}

DistanceInformer::~DistanceInformer()
{
    if(universe() != nullptr)
    {
        universe()->removeFigure(_line);
    }
}

void DistanceInformer::setVisible(bool visible)
{
    if(visible != isVisible())
    {
        _isVisible = visible;

        if(_line)
            _line->setVisible(visible);
        if(_label)
            _label->setVisible(visible);
    }
}

void DistanceInformer::setColor(const QColor &c)
{
    if(c != color())
    {
        _color = c;

        if(_line)
        {
            _line->setColor(_color);
        }
        redrawText();
    }
}

void DistanceInformer::setPoints(const QVector3D &first, const QVector3D &second, const QColor &c)
{
    if(isVisible())
    {
        if(c.isValid() && c != color())
        {
            _color = c;
        }
        if(_line)
        {
            _line->createLine(first, second, color());
        }

        if(_label)
        {
            QVector3D pos((first.x() + second.x()) * 0.5f,
                          (first.y() + second.y()) * 0.5f,
                          (first.z() + second.z()) * 0.5f);
            _label->setCoordinates(pos);

            _value = first.distanceToPoint(second);

            generateText();
        }
    }
}

void DistanceInformer::generateText()
{
    QString text = QString::number(_value, 'f', _precision) + _suffix;
    if(!_postfix.isEmpty())
        text = text + "\n" + _postfix;
    if(text == _valueText)
        return;

    _valueText = text;
    redrawText();
}

void DistanceInformer::redrawText()
{
    drawText(true);
}

}
