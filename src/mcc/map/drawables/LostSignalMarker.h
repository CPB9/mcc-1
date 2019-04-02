#pragma once

#include "mcc/Config.h"
#include "mcc/map/drawables/Point.h"
#include "mcc/map/drawables/Label.h"
#include "mcc/map/Fwd.h"
#include "mcc/geo/Fwd.h"

#include <chrono>

class QPainter;
class QPointF;

namespace mccmap {

class MCC_MAP_DECLSPEC LostSignalMarker : public Point {
public:
    LostSignalMarker(const mccgeo::LatLon& latLon, const std::chrono::system_clock::time_point& time,
                     const MapRect* mapRect, const mccui::CoordinateSystemController* csController);
    LostSignalMarker(const QPointF& position, const mccgeo::LatLon& latLon,
                     const std::chrono::system_clock::time_point& time, const mccui::CoordinateSystemController* csController);
    ~LostSignalMarker();

    void draw(QPainter* p, const QPixmap& lostSignalPixmap, const MapRect* rect) const;
    inline const std::chrono::system_clock::time_point& time() const;

private:
    void update(const mccgeo::LatLon& latLon, const std::chrono::system_clock::time_point& time, const mccui::CoordinateSystemController* csController);
    LabelBase _label;
    std::chrono::system_clock::time_point _time;
};

inline const std::chrono::system_clock::time_point& LostSignalMarker::time() const
{
    return _time;
}
}
