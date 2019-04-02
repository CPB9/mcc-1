#include "mcc/ide/models/DevicesListModel.h"

#include <QAbstractListModel>
#include "mcc/uav/UavController.h"

namespace mccide {

using mccuav::UavController;
using mccuav::Uav;

DevicesListModel::~DevicesListModel()
{
}

DevicesListModel::DevicesListModel(mccuav::UavController* uavController, QObject* parent)
    : QAbstractListModel(parent)
    , _uavController(uavController)
{
    connect(_uavController, &UavController::uavAdded, this, &DevicesListModel::resetModel);
    connect(_uavController, &UavController::uavRemoved, this, &DevicesListModel::resetModel);
    connect(_uavController, &UavController::uavFirmwareLoaded, this, &DevicesListModel::resetModel);
}

int DevicesListModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return (int)_devices.size();
}

QVariant DevicesListModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::UserRole)
        return _devices[index.row()].toQUuid();

    if (role == Qt::DisplayRole)
    {
        auto dev = _uavController->uav(_devices[index.row()]);
        if (dev.isNone())
            return QVariant();

        return QString("%1(%2)")
            .arg(dev->getInfo())
            .arg(dev->deviceId());
    }
    return QVariant();
}

bool DevicesListModel::update(const mccmsg::Devices& devices)
{
    if (_devices != devices)
    {
        _devices = devices;
        resetModel();
        return true;
    }
    return false;
}

void DevicesListModel::resetModel()
{
    beginResetModel();
    endResetModel();
}

bool DevicesListModel::hasDevice(const mccmsg::Device& name) const
{
    return std::find(_devices.begin(), _devices.end(), name) != _devices.end();
}
}
