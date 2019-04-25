#pragma once
#include "mcc/Config.h"
#include <bmcl/Option.h>
#include <bmcl/Result.h>
#include "mcc/msg/Objects.h"
#include <QString>
#include <QDataStream>
#include <QMetaType>
#include <assert.h>

class QMimeData;

namespace mccuav {

class MCC_UAV_DECLSPEC PlotData
{
public:
    PlotData();
    PlotData(const mccmsg::Device& device, const std::string& trait, const std::string& varId, const std::string& description);

    const mccmsg::Device& device() const;
    const std::string& trait() const;
    const std::string& varId() const;
    const std::string& description() const;

    static QMimeData* packMimeData(PlotData* data, const QString& mimeTypeStr);
    static bmcl::Option<PlotData*> unpackMimeData(const QMimeData* data, const QString& mimeStr);
    static const char* mimeDataStr();

    bool operator==(const PlotData& other);

    friend QDataStream & operator << (QDataStream &out, const PlotData& obj)
    {
        out << obj.device().toQString();
        out << QString::fromStdString(obj.trait());
        out << QString::fromStdString(obj.varId());
        out << QString::fromStdString(obj.description());
        return out;
    }

    friend QDataStream & operator >> (QDataStream &in, PlotData& obj)
    {
        QString deviceStr;
        QString trait;
        QString varId;
        QString description;

        in >> deviceStr >> trait >> varId >> description;
        auto res = mccmsg::Device::createFromString(deviceStr);
        if(res.isOk())
        {
            obj._device = (mccmsg::Device)res.take();
            obj._trait = trait.toStdString();
            obj._varId = varId.toStdString();
            obj._description = description.toStdString();
        }
        else
        {
            assert(false);
        }
        return in;
    }
private:
    mccmsg::Device _device;
    std::string _trait;
    std::string _varId;
    std::string _description;
};

}

Q_DECLARE_METATYPE(mccuav::PlotData);
