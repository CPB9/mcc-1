#pragma once
#include "mcc/Config.h"
#include "mcc/Rc.h"
#include "mcc/msg/Objects.h"
#include <string>

namespace mccmsg {

class MCC_MSG_DECLSPEC RadarDescriptionObj : public mcc::RefCountable
{
public:
    RadarDescriptionObj(const Radar& name, bmcl::StringView info, bmcl::StringView settings);
    ~RadarDescriptionObj() override;
    const Radar& name() const;
    const std::string& info() const;
    const std::string& settings() const;
private:
    Radar       _name;
    std::string _info;
    std::string _settings;
};

using Radars = std::vector<Radar>;
using RadarDescription = bmcl::Rc<const RadarDescriptionObj>;
using RadarDescriptions = std::vector<RadarDescription>;
}
