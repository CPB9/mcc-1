#pragma once

#include <QPointF>
#include <QLineF>
#include <QRectF>

#include <vector>
#include <utility>

namespace mccgeo {

using Point = QPointF;

typedef std::vector<Point> PointVector;

class Interval {
public:
    Interval(double start, double end)
        : _start(start)
        , _end(end)
    {
    }

    double start() const
    {
        return _start;
    }

    double end() const
    {
        return _end;
    }

private:
    double _start;
    double _end;
};

typedef std::vector<Interval> IntervalVector;

template<typename T>
struct Directed {
    template<typename... A>
    Directed(double direction, A&&... args)
        : direction(direction)
        , value(std::forward<A>(args)...)
    {
    }

    double direction;
    T value;
};

template<typename T>
using DirectedVector = std::vector<Directed<T>>;

using Line = QLineF;
using Rect = QRectF;
}
