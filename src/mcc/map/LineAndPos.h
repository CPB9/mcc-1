#pragma once

#include <QPointF>

#include <cstddef>

namespace mccmap {

struct LineIndexAndPos {
    LineIndexAndPos(std::size_t index, const QPointF& pos)
        : index(index)
        , pos(pos)
    {
    }

    std::size_t index;
    QPointF pos;
};
}
