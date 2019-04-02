#pragma once

#include <QObject>
#include <mcc/uav/Uav.h>

namespace mccqml {

class MCC_QML_DECLSPEC QmlDeviceGroup : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<QObject*> devices READ getDevices NOTIFY devicesChanged)
    Q_PROPERTY(int groupId MEMBER _groupId)

signals:
    void devicesChanged();

public:

    QmlDeviceGroup(int groupId)
        : _groupId(groupId)
    {

    }

    ~QmlDeviceGroup()
    {
    }

    void addDevice(mccuav::Uav* uav)
    {
        using mccuav::Uav;

        _devices.push_back(uav);

        std::stable_sort(_devices.begin(), _devices.end(),
                         [](Uav* left, Uav* right)
                        {
                            if (left->isActivated() > right->isActivated())
                                return true;
                            else if (left->isActivated() < right->isActivated())
                                return false;

                            if (left->protocol() < right->protocol())
                                return true;
                            else if (left->protocol() != right->protocol())
                                return false;

                            return left->deviceId() < right->deviceId();
                        }
        );
    }

    QList<QObject*> getDevices()
    {
        QList<QObject*> res;
        for (auto d : _devices)
            res.push_back(d);
        return res;
    }
private:
    int _groupId;
    QList<mccuav::Uav*> _devices;
};
}
