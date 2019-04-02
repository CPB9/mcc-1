#include <bmcl/StringView.h>
#include "mcc/msg/obj/Radar.h"

namespace mccmsg {

RadarDescriptionObj::RadarDescriptionObj(const Radar& name, bmcl::StringView info, bmcl::StringView settings)
    : _name(name), _info(info.toStdString()), _settings(settings.toStdString())
{
}
RadarDescriptionObj::~RadarDescriptionObj() {}

const Radar& RadarDescriptionObj::name() const { return _name; }
const std::string& RadarDescriptionObj::info() const { return _info; }
const std::string& RadarDescriptionObj::settings() const { return _settings; }


}
