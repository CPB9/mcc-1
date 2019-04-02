#include "mcc/ui/GraphicsEffectCreator.h"

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsColorizeEffect>
#include <QPainter>

namespace mccui {

QPixmap GraphicsEffectCreator::applyColorEffect(const QPixmap& src, QColor color, float strength /*= 1.0*/, float extent /*= 0*/)
{
    QGraphicsColorizeEffect* effect = new QGraphicsColorizeEffect();
    effect->setStrength(strength);
    effect->setColor(color);

    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(src);
    item.setGraphicsEffect(effect);
    scene.addItem(&item);
    QImage res(src.size() + QSize(extent * 2, extent * 2), QImage::Format_ARGB32_Premultiplied);
    res.fill(Qt::transparent);
    QPainter ptr(&res);
    scene.render(&ptr, QRectF(), QRectF(-extent, -extent, src.width() + extent * 2, src.height() + extent * 2));
    return QPixmap::fromImage(res);
}
}
