#include "../widgets/FirmwareModel.h"
#include "mcc/msg/ptr/Firmware.h"
#include "mcc/msg/ptr/Tm.h"
#include "mcc/msg/ParamList.h"

#include <bmcl/DoubleEq.h>
#include <QColor>

namespace mccmav {

FirmwareModel::FirmwareModel(QObject* parent)
    : QAbstractTableModel(parent)
{

}

int FirmwareModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    if (_firmware.isNull())
        return 0;

    return _params.size();
}

int FirmwareModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return 3;
}

QVariant FirmwareModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() > _params.size())
        return QVariant();

    const auto& desc = _params.at(index.row());
    auto param = _values[index.row()];

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case 0:
            return QString::fromStdString(desc.name);
        case 1:
            if (param.isSome())
            {
                if (param.isInt() || param.isUint())
                {
                    int refValue = param.toInt();
                    auto it = desc.values.find(refValue);
                    if (it != desc.values.end())
                        return QString::fromStdString(it->second);
                }

                return param.qstringify();
            }
            else
                return "<undefined>";
        case 2:
            return QString::fromStdString(desc.shortDesc);
        }
    }
    if (role == Qt::TextColorRole)
    {
        if (param.isSome() && !bmcl::doubleEq(param.toDouble(), desc.defaultValue))
            return QColor(Qt::yellow);
        return QColor(Qt::white);
    }

    if (role == Qt::UserRole)
    {
        return QVariant::fromValue(desc);
    }

    if (role == Qt::UserRole + 1)
    {
        return param.toQVariant();
    }

    return QVariant();
}

void FirmwareModel::setFirmware(const FirmwarePtr& firmware)
{
    beginResetModel();
    _firmware = firmware;
    _params = firmware->paramsDescription();
    for (size_t i = 0; i < _params.size(); ++i)
        _values.push_back(mccmsg::NetVariant());

    endResetModel();
}

FirmwarePtr FirmwareModel::firmware() const
{
    return _firmware;
}

void FirmwareModel::setParam(const ParamValue& param)
{
    if (_firmware.isNull())
        return;

    if (_values.empty())
        return;
    if (param.index > _values.size() || param.index < 0)
    {
        assert(false);
        return;
    }

    _values[param.index] = param.value;
    emit dataChanged(index(param.index, 0), index(param.index, columnCount() - 1));
}

void FirmwareModel::setTmStorage(const bmcl::Rc<mccmsg::ITmStorage>& v)
{
    _tmStorage = bmcl::dynamic_pointer_cast<TmStorage>(v);
    if (_tmStorage.isNull())
        return;
    setFirmware(_tmStorage->firmware().unwrap());
    _tmStorage->namedAccess().addHandler([this](const ParamValue& p) { setParam(p); });
}

}
