#pragma once
#include "mcc/Config.h"
#include <bmcl/Option.h>
#include <QVector>
#include <QMetaType>

namespace mccuav {

struct UavMotionLimits
{
    bmcl::Option<double> minVelocity;
    bmcl::Option<double> maxVelocity;
    bmcl::Option<double> minAltitude;
    bmcl::Option<double> maxAltitude;
};

enum class TrackMode { All, Distance, Time, None };

class MCC_UAV_DECLSPEC TrackSettings
{
public:
    TrackSettings();
    TrackSettings(const TrackSettings& other);
    TrackMode mode() const;
    TrackMode lastMode() const;
    void setMode(TrackMode mode);
    int value() const;
    void setValue(int value);
    int seconds() const;
    int meters() const;
    void setSeconds(int seconds);
    void setMeters(int meters);
private:
    bool        _show;
    TrackMode   _mode;
    int         _seconds;
    int         _meters;
};
}

Q_DECLARE_METATYPE(mccuav::TrackMode);