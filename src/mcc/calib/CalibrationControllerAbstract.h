#pragma once

#include <QWidget>

#include <bmcl/Logging.h>

#include "mcc/msg/Calibration.h"
#include "mcc/uav/UavController.h"

namespace mcccalib {

struct VarCondition
{
    using Condition = std::function<bool(const mccmsg::NetVariant& value)>;

    std::string           id;
    mccmsg::NetVariant value;
    Condition             func;

    VarCondition(const std::string& id, Condition condition)
        : id(id), func(condition)
    {

    }
};

using VarConditions = std::vector<VarCondition>;

class CalibrationControllerAbstract : public QWidget
{
    Q_OBJECT

public:
    CalibrationControllerAbstract(mccmsg::CalibrationSensor sensor)
        : _calibrated(false), _sensor(sensor)
    {
//         connect(mccContext, &mccui::Context::systemStarted, this,
//             [this]()
//             {
//                 connect(mccContext->exchangeService(), &mccuav::ExchangeService::traitCommonCalibrationStatus, this, &CalibrationControllerAbstract::onCommonCalibrationStatus);
//             }
//         );
    }

    virtual bool calibrated() const { return _calibrated; }
    mccmsg::CalibrationSensor sensor() const { return _sensor; }
    const mccmsg::Device& device() const
    {
        return _device;
    }
    void setDevice(const mccmsg::Device& device)
    {
        _device = device;
    }

    virtual void start() = 0;
    virtual void cancel() = 0;
    virtual void onTraitCalibrationState(const mccmsg::TmCalibrationPtr& msg) {};

    void onCommonCalibrationStatus(const mccmsg::TmCommonCalibrationStatusPtr& status)
    {
        if (_device != status->device())
            return;

        bool newCalibrated = status->isSensorCalibrated(_sensor);
        if (newCalibrated != _calibrated)
        {
            _calibrated = newCalibrated;
            emit calibratedChanged();
        }
    }
signals:
    void calibratedChanged();
    void cancelled();
    void completed();
    void failed();

private:
    mccmsg::CalibrationSensor  _sensor;
    mccmsg::Device             _device;
    bool                        _calibrated;
};
}
