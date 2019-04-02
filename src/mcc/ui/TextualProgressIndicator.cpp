#include "mcc/ui/TextualProgressIndicator.h"

#include <QFontMetricsF>
#include <QPainter>

namespace mccui{

TextualProgressIndicator::TextualProgressIndicator(QWidget *parent)
    : QProgressIndicator(parent)
    , _text()
    , _textColor(color())
{}

TextualProgressIndicator::~TextualProgressIndicator() {}

void TextualProgressIndicator::paintEvent(QPaintEvent* event)
{
    QProgressIndicator::paintEvent(event);

    if(text().isEmpty())
        return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(_textColor));
    p.drawText(rect(), Qt::AlignCenter, _text);
}

void TextualProgressIndicator::resizeEvent(QResizeEvent* event)
{
    QProgressIndicator::resizeEvent(event);

    adjustTextSize();
}

void TextualProgressIndicator::adjustTextSize()
{
    if(text().isEmpty())
        return;

    QFontMetricsF metrics(font());
    QRectF bRect = metrics.boundingRect(text());
    qreal factor = std::min(static_cast<qreal>(width())/bRect.width(),
                            static_cast<qreal>(height())/bRect.height());
    QFont font = this->font();
    font.setPointSizeF(font.pointSizeF() * factor);
    setFont(font);
}

void TextualProgressIndicator::setTextColor(const QColor& textColor)
{
    if(_textColor == textColor)
        return;

    _textColor = textColor;
    update();
}

void TextualProgressIndicator::setColor(const QColor& color)
{
    if(this->color() == _textColor || !textColor().isValid())
        setTextColor(color);

    QProgressIndicator::setColor(color);
}

void TextualProgressIndicator::setText(const QString& text)
{
    if(_text == text)
        return;

    _text = text;
    adjustTextSize();
    update();
}
}
