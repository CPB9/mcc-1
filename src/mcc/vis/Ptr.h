#pragma once

#include "mcc/vis/Rc.h"
#include <memory>

namespace mccvis {

class Radar;
class RadarGroup;

typedef Rc<Radar> RadarPtr;
typedef std::shared_ptr<RadarGroup> RadarGroupPtr;
}
