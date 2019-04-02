#pragma once

namespace mccmap {

struct TilePosition {
    TilePosition(int z, int x, int y)
    {
        zoomLevel = z;
        globalOffsetX = x;
        globalOffsetY = y;
    }

    TilePosition()
        : TilePosition(1, 0, 0)
    {
    }

    int zoomLevel;
    int globalOffsetX;
    int globalOffsetY;
};

inline bool operator==(const TilePosition& left, const TilePosition& right)
{
    return left.zoomLevel == right.zoomLevel && left.globalOffsetX == right.globalOffsetX
        && left.globalOffsetY == right.globalOffsetY;
}
}
