#pragma once
#include <string>
#include <vector>
#include <algorithm>

#include "mcc/msg/Fwd.h"
#include "mcc/msg/TmView.h"
#include "mcc/msg/NetVariant.h"
#include "mcc/msg/obj/Firmware.h"

#include "device/Mavlink.h"
#include "device/MavlinkUtils.h"

namespace mccmav {

class Firmware;
using FirmwarePtr = bmcl::Rc<const Firmware>;

struct ParamValue
{
    ParamValue() = default;
    uint8_t             componentId;
    std::string         name;
    MAV_PARAM_TYPE      type;
    int                 index;
    std::string         trait;
    mccmsg::NetVariant value;
};

struct ParameterDescription
{
    ParameterDescription() = default;
    //uint8_t             componentId;
    int                 index;
    std::string         name;
    std::string         shortDesc;
    std::string         longDesc;
    std::string         type;
    std::string         unit;
    std::string         category;
    double              defaultValue;
    double              min;
    double              max;
    std::map<int, std::string> values;
};

class Firmware : public mccmsg::IFirmware
{
public:
    Firmware(const mccmsg::ProtocolValue& id, MAV_AUTOPILOT autopilot, const std::vector<ParamValue>& params, const mccmsg::PropertyDescriptionPtrs& req, const mccmsg::PropertyDescriptionPtrs& opt);
    Firmware(const mccmsg::ProtocolValue& id, MAV_AUTOPILOT autopilot, const std::vector<ParameterDescription>& params, const mccmsg::PropertyDescriptionPtrs& req, const mccmsg::PropertyDescriptionPtrs& opt);
    ~Firmware();
    const std::vector<ParameterDescription>& paramsDescription() const;
    bmcl::Buffer encode() const override;
    static bmcl::OptionRc<const Firmware> decode(const mccmsg::ProtocolValue& id, bmcl::Bytes bytes);

    bool isPx4() const;
private:
    void loadXml(const std::vector<ParamValue>& params);
private:
    MAV_AUTOPILOT _autopilotBoard;
    std::vector<ParameterDescription> _paramsDescription;
};

}

Q_DECLARE_METATYPE(mccmav::ParameterDescription)
Q_DECLARE_METATYPE(mccmav::ParamValue)
