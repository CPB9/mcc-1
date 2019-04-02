#pragma once

#include "mcc/vis/Config.h"
#include "mcc/vis/Rc.h"
#include "mcc/vis/Point.h"
#include "mcc/vis/Profile.h"

#include <vector>

namespace mccvis {

class MCC_VIS_DECLSPEC Region : public RefCountable {
public:
    Region(const std::vector<Rc<Profile>>& slices, const ViewParams& params);
    Region(std::vector<Rc<Profile>>&& slices, const ViewParams& params);
    ~Region();

    const std::vector<Rc<Profile>>& profiles() const;
    const std::vector<PointVector>& curves() const;
    const std::vector<PointVector>& hitCurves() const;
    double maxDistance() const;
    const ViewParams& params() const;
    double viewCoeff() const;
    double hitCoeff() const;

private:
    void updateData();

    std::vector<PointVector> _curves;
    std::vector<PointVector> _hitCurves;
    std::vector<Rc<Profile>> _profiles;
    double _maxDistance;
    ViewParams _params;
    double _viewCoeff;
    double _hitCoeff;
};
}
