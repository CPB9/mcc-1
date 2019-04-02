#pragma once

#include "mcc/Config.h"
#include "mcc/map/drawables/Drawable.h"
#include "mcc/map/drawables/Movable.h"
#include "mcc/map/drawables/WithRect.h"
#include "mcc/map/drawables/WithPosition.h"
#include "mcc/map/drawables/Zoomable.h"

namespace mccmap {

template <typename B>
class AbstractShape : public Drawable<B>, public WithRect<B>, public Movable<B>, public Zoomable<B> {
};

template <typename B>
class AbstractMarker : public AbstractShape<B>, public WithPosition<B> {
};

template <typename B>
class AbstractPolyLine : public AbstractShape<B> { //, public WithPoints<B> {
};
}
