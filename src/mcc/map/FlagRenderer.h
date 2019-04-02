#pragma once

#include "mcc/Config.h"

#include <cstddef>
#include <cstdint>

#include "mcc/msg/Property.h"

class QPainter;
class QImage;
class QString;

class QRectF;
class QColor;

namespace mccmap {

class MCC_MAP_DECLSPEC FlagRenderer {
public:
    static QImage renderFlag(std::size_t width, std::size_t height, const QColor& color);
    static QImage renderWaypointFlag(std::size_t width, std::size_t height, const QColor& color,
                                     std::size_t waypointNumber, const mccmsg::PropertyValues& waypointFlags);
    static QImage renderWaypointOverlayFlag(std::size_t width, std::size_t height, std::size_t waypointNumber,
                                            const mccmsg::PropertyValues& waypointFlags);

    static QImage renderWaypointFlag(std::size_t width, std::size_t height, const QColor& color);
};
}
