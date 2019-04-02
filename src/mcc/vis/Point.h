#pragma once

#include "mcc/vis/Config.h"
#include "mcc/geo/Point.h"

namespace mccvis {

using Point = mccgeo::Point;
using PointVector = mccgeo::PointVector;
using Interval = mccgeo::Interval;
using Line = mccgeo::Line;
using Rect = mccgeo::Rect;

using IntervalVector = mccgeo::IntervalVector;
template <typename T>
using Directed = mccgeo::Directed<T>;
template <typename T>
using DirectedVector = mccgeo::DirectedVector<T>;
}
