#include "mcc/uav/PlotData.h"

#include <bmcl/Buffer.h>
#include <bmcl/MemReader.h>
#include <bmcl/FileUtils.h>
#include <bmcl/Result.h>

#include "mcc/msg/obj/Device.h"

#include <QMimeData>

namespace mccuav {

PlotData::PlotData(const mccmsg::Device& device, const std::string& trait, const std::string& varId, const std::string& description)
    : _device(device), _trait(trait), _varId(varId), _description(description)
{

}

PlotData::PlotData()
{

}

const mccmsg::Device& PlotData::device() const
{
    return _device;
}

const std::string& PlotData::trait() const
{
    return _trait;
}

const std::string& PlotData::varId() const
{
    return _varId;
}

const std::string& PlotData::description() const
{
    return _description;
}

QMimeData* PlotData::packMimeData(PlotData* data, const QString& mimeTypeStr)
{
    QMimeData* mdata = new QMimeData;

    bmcl::Buffer buf;
    buf.writeUint64Le(bmcl::applicationPid());
    buf.writeUint64Le((uint64_t)data);
    mdata->setData(mimeTypeStr, QByteArray((const char*)buf.begin(), buf.size()));
    return mdata;
}

bmcl::Option<mccuav::PlotData*> PlotData::unpackMimeData(const QMimeData* data, const QString& mimeStr)
{
    QByteArray d = data->data(mimeStr);
    if(d.size() != 16)
    {
        return bmcl::None;
    }
    bmcl::MemReader reader((const uint8_t*)d.data(), d.size());
    if(reader.readUint64Le() != bmcl::applicationPid())
    {
        return bmcl::None;
    }
    uint64_t addr = reader.readUint64Le(); //unsafe
    return (PlotData*)addr;

}

const char* PlotData::mimeDataStr()
{
    return "mcc/plot_data";
}

bool PlotData::operator==(const PlotData& other)
{
    return device() == other.device() && trait() == other.trait() && varId() == other.varId() && description() == other.description();
}


}
