#include "mcc/map/drawables/LostSignalMarker.h"
#include "mcc/map/drawables/Label.h"
#include "mcc/ui/CoordinateSystemController.h"

#include <QPainter>

#include <iomanip>
#include <ctime>

namespace mccmap {

LostSignalMarker::LostSignalMarker(const mccgeo::LatLon& latLon,
                                   const std::chrono::system_clock::time_point& time,
                                   const MapRect* mapRect, const mccui::CoordinateSystemController* csController)
    : Point(WithPosition<>::fromLatLon(latLon, mapRect))
{
    update(latLon, time, csController);
}

LostSignalMarker::LostSignalMarker(const QPointF& position,
                                   const mccgeo::LatLon& latLon,
                                   const std::chrono::system_clock::time_point& time,
                                   const mccui::CoordinateSystemController* csController)
    : Point(position)
{
    update(latLon, time, csController);
}

LostSignalMarker::~LostSignalMarker()
{
}

void LostSignalMarker::update(const mccgeo::LatLon& latLon, const std::chrono::system_clock::time_point& time, const mccui::CoordinateSystemController* csController)
{
    _time = time;
    _label.setLabelAlignment(Qt::AlignBottom | Qt::AlignLeft);
    std::time_t a = std::chrono::system_clock::to_time_t(time);
    char output[13];
    std::strftime(output, sizeof(output), " (%H:%M:%S)", std::localtime(&a));
    QString coords = csController->convertAndFormat(latLon, "%1, %2");
    coords.append(output);
    _label.setLabel(coords);
}

void LostSignalMarker::draw(QPainter* p, const QPixmap& lostSignalPixmap, const MapRect* rect) const
{
    double height = lostSignalPixmap.height();
    double halfWidth = lostSignalPixmap.width() / 2;
    p->drawPixmap(position() + QPointF(-halfWidth, -height) - rect->mapOffsetRaw(), lostSignalPixmap);
    _label.drawAt(p, position() + QPointF(halfWidth, -height) - rect->mapOffsetRaw());
}
}
