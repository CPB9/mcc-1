#pragma once

#include <cstddef>

namespace mccmap {

class WebMapProperties {
public:
    static constexpr std::size_t tilePixelSize()
    {
        return 256;
    }

    static constexpr int minZoom()
    {
        return 0;
    }

    static constexpr int maxZoom()
    {
        return 22;
    }
};
}
