#pragma once

#include "mcc/geo/Config.h"
#include "mcc/geo/Position.h"
#include "mcc/geo/Vector3D.h"

#include <vector>

namespace mccgeo
{

class GroupGeometry;

class MCC_GEO_DECLSPEC EnuPositionHandler
{
public:
    explicit EnuPositionHandler();
    ~EnuPositionHandler();

    Vector3D toEnuPosition(const Position& pos, const Position& p0 = Position());  // returns local coords of points [p0 + ps] relative projection p0 on eath surface.
    std::vector<Vector3D> toEnuPosition(const std::vector<Position>& ps, const Position& p0 = Position());

    Position toGpsPosition(const Vector3D& pos, const Position& gps0);
    std::vector<Position> toGpsPosition(const std::vector<Vector3D>& ps, const Position& gps0);

    GroupGeometry calcGroupParams(const std::vector<Position>& poss, double time);

private:
    double      _lastMoment;
    Vector3D    _lastGravityCenterPosition;
};

}
