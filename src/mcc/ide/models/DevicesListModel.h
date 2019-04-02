#pragma once

#include <QAbstractListModel>
#include <QString>

#include "mcc/Config.h"
#include "mcc/msg/obj/Device.h"
#include "mcc/uav/Fwd.h"

namespace mccide {

class MCC_IDE_DECLSPEC DevicesListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    DevicesListModel(mccuav::UavController* uavController, QObject* parent);
    ~DevicesListModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    bool update(const mccmsg::Devices& devices);

private slots:
    void resetModel();

private:
    bool hasDevice(const mccmsg::Device& name) const;

private:
    mccuav::UavController* _uavController;
    mccmsg::Devices _devices;
};
}
