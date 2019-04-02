#pragma once
#include "mcc/Config.h"
#include <bmcl/Option.h>
#include "mcc/msg/Objects.h"
#include <QString>

class QMimeData;

namespace mccuav {

class MCC_UAV_DECLSPEC PlotData
{
public:
    PlotData(const mccmsg::Device& device, const std::string& trait, const std::string& varId, const std::string& description);

    const mccmsg::Device& device() const;
    const std::string& trait() const;
    const std::string& varId() const;
    const std::string& description() const;

    static QMimeData* packMimeData(PlotData* data, const QString& mimeTypeStr);
    static bmcl::Option<PlotData*> unpackMimeData(const QMimeData* data, const QString& mimeStr);
    static const char* mimeDataStr();

private:
    mccmsg::Device _device;
    std::string _trait;
    std::string _varId;
    std::string _description;
};

}
