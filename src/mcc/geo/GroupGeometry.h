#pragma once

#include "mcc/geo/Config.h"
#include "mcc/geo/Position.h"
#include "mcc/geo/Vector3D.h"

#include <chrono>
#include <utility>

namespace mccgeo
{

class MCC_GEO_DECLSPEC GroupGeometry
{
public:
    GroupGeometry(const Position& pos, const Vector3D& vec, double maxRem, double minConv, int index1, int index2);
    ~GroupGeometry();

    Position gravityCenterPosition() const;
    Vector3D gravityCenterVelocity() const;
    double maxRemotenessFromCenter() const;
    double minConvergence() const;
    std::chrono::system_clock::time_point time() const;

private:
    Position            _gravityCenterPosition;
    Vector3D            _gravityCenterVelocity;
    double              _maxRemotenessFromCenter;
    double              _minConvergence;
    std::pair<int, int> _minConvergenceIndexes;
    std::chrono::system_clock::time_point _time;
};

}
