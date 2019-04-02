#include "mcc/map/drawables/Label.h"

#include <QImage>
#include <QPainter>
#include <QApplication>

namespace mccmap {

LabelBase::LabelBase(const QString& label)
    : _label(label)
    , _background(Qt::white)
    , _labelScale(1)
    , _alignment(Qt::AlignCenter)
{
}

LabelBase::~LabelBase()
{
}

void LabelBase::drawAt(QPainter* p, const QPointF& pos) const
{
    p->drawPixmap(pos + _offset, _renderedLabel);
}

void LabelBase::setLabel(const QString& label)
{
    _label = label;
    repaintLabelPixmap();
}

void LabelBase::setLabelScale(double scale)
{
    _labelScale = scale;
    repaintLabelPixmap();
}

void LabelBase::setLabelBackground(const QColor& color)
{
    _background = color;
    repaintLabelPixmap();
}

void LabelBase::setLabelAlignment(Qt::Alignment alignment)
{
    _alignment = alignment;
    repaintLabelPixmap();
}

void LabelBase::setLabelParams(const QString& label, const QColor& color, double scale)
{
    _label = label;
    _background = color;
    _labelScale = scale;
    repaintLabelPixmap();
}

QSize LabelBase::computeSize(const QString& text)
{
    return QFontMetrics(QApplication::font()).size(0, text);
}

QImage LabelBase::createLabelImage(const QString& text, const QColor& color)
{
    QPainter p;
    QSize size = QFontMetrics(QApplication::font()).size(0, text) + QSize(4, 2);
    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    p.begin(&img);
    QRectF fillRect(QPoint(0, 0), size);
    QRectF textRect(QPoint(2, 1), size);
    if (!text.isEmpty())
    {
        p.fillRect(fillRect, color);
        p.drawText(textRect, text);
    }
    p.end();
    return img;
}

void LabelBase::repaintLabelPixmap()
{
    QFont font = QApplication::font();
    QSize size = QFontMetrics(font).size(0, _label) + QSize(4, 2);

    font.setPointSizeF(font.pointSizeF() * _labelScale);
    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    if (!size.isEmpty()) {
        img.fill(Qt::transparent);

        if (size.width() != 0 || size.height() != 0) {
            QPainter p;
            p.begin(&img);
            p.setFont(font);
            QRectF fillRect(QPoint(0, 0), size);
            QRectF textRect(QPoint(2, 1), size);
            if (!_label.isEmpty())
            {
                p.fillRect(fillRect, _background);
                p.drawText(textRect, _label);
            }
            p.end();
        }
    }
    _renderedLabel = QPixmap::fromImage(img);
    _offset = WithPosition<>::edgePoint(img.size(), _alignment);
}

void LabelBase::drawLabelAt(QPainter* p, const QString& text, const QPointF& point, const QColor& color)
{
    QSize size = QFontMetrics(QApplication::font()).size(0, text);
    QRectF rect(point, size);
    if (!text.isEmpty())
    {
        p->fillRect(rect, color);
        p->drawText(rect, text);
    }
}

void LabelBase::drawLabelAt(QPainter* p, const QString& text, const QPointF& point, const QColor& color,
                            Qt::Alignment alignment, double scale)
{
    QFont oldFont = p->font();
    QFont font = QApplication::font();
    font.setPointSizeF(font.pointSizeF() * scale);
    QSize size = QFontMetrics(font).size(0, text) + QSize(4, 2);
    QPointF offset = WithPosition<>::edgePoint(size, alignment);
    QRectF fillRect(point + offset, size);
    QRectF textRect(point + offset + QPoint(2, 1), size);
    p->setFont(font);
    if (!text.isEmpty())
    {
        p->fillRect(fillRect, color);
        p->drawText(textRect, text);
    }
    p->setFont(oldFont);
}

Label::Label(const QPointF& position, const QString& label)
    : LabelBase(label)
    , _point(position)
{
}

Label::~Label()
{
}

void Label::draw(QPainter* p, const MapRect* rect) const
{
    p->drawPixmap(_point.position() + _offset - rect->mapOffsetRaw(), labelPixmap());
}
}
