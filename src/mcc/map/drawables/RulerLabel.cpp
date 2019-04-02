#include "mcc/map/drawables/RulerLabel.h"
#include "mcc/map/drawables/Label.h"

#include <bmcl/Math.h>

#include <QPainter>

#include <cmath>

namespace mccmap {

void RulerLabel::draw(QPainter* p, const MapRect* rect) const
{
    if (!_isVisible) {
        return;
    }
    QTransform t = p->transform();
    // p->rotate(_rotation);
    p->drawPixmap((_point.position() + _offset - rect->mapOffsetRaw()).toPoint(), _pixmap);
    p->setTransform(t);
}

void RulerLabel::setDistanceAndRotation(double distance, double rotation)
{
    _rotation = rotation;
    const char* suffix = "м";
    if (distance > 1000) {
        distance /= 1000;
        suffix = "км";
    }
    QString az = mccui::CoordinateSystemController::formatValue(std::trunc(std::fmod(90 - rotation + 360, 360)),
                                                       mccui::CoordinateFormat(mccui::AngularFormat::Degrees),
                                                       0);
    if (rotation > 90) {
        rotation -= 180;
    } else if (rotation < -90) {
        rotation += 180;
    }
    QString text = (QString("%1%2 %3")).arg(distance, 0, 'f', 2).arg(suffix).arg(az);
    QTransform t;
    t.rotate(-rotation);
    double radians = bmcl::degreesToRadians(-rotation + 90);
    QImage img = Label::createLabelImage(text, Qt::white);
    int labelOffset = img.height() * 1.1;
    _pixmap = QPixmap::fromImage(img.transformed(t, Qt::SmoothTransformation));
    _offset = -_pixmap.rect().center() - labelOffset * QPointF(std::cos(radians), std::sin(radians));
}
}
