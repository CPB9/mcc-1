#include "mcc/geo/GroupGeometry.h"

namespace mccgeo
{

GroupGeometry::GroupGeometry(const Position& pos, const Vector3D& vec,
                             double maxRem, double minConv, int index1,
                             int index2)
        : _gravityCenterPosition(pos)
        , _gravityCenterVelocity(vec)
        , _maxRemotenessFromCenter(maxRem)
        , _minConvergence(minConv)
        , _minConvergenceIndexes(index1, index2)
{
    _time = std::chrono::system_clock::now();
}

GroupGeometry::~GroupGeometry() {}

Position GroupGeometry::gravityCenterPosition() const { return _gravityCenterPosition;}
Vector3D GroupGeometry::gravityCenterVelocity() const { return _gravityCenterVelocity; }
double GroupGeometry::maxRemotenessFromCenter() const { return _maxRemotenessFromCenter; }
double GroupGeometry::minConvergence() const { return _minConvergence; }
std::chrono::system_clock::time_point GroupGeometry::time() const { return _time; }

}
