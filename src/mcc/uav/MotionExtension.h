#pragma once

#include <QObject>

#include <bmcl/Option.h>

#include "mcc/Config.h"

namespace mccuav
{

struct MotionIndicatorSettings
{
    float catchDelta;
    bmcl::Option<float> greenLimit;
    bmcl::Option<float> yellowLimit;
};

class MCC_UAV_DECLSPEC MotionExtension : public QObject
{
    Q_OBJECT

public:
    MotionExtension(QObject* parent);

    void setVelocitySource(const bmcl::Option<QString>& source);
    void setVelocityReference(const bmcl::Option<float>& velocity);
    void setVelocityUnits(const bmcl::Option<QString>& units);
    void setVelocitySettings(const MotionIndicatorSettings& settings);

    void setAltitudeSource(const bmcl::Option<QString>& source);
    void setAltitudeReference(bmcl::Option<float> source);
    void setShowScale(bool showScale);
    void setAltitudeSettings(const MotionIndicatorSettings& settings);

    void setRollReference(const bmcl::Option<float>& roll);

    void setHeadingSource(const bmcl::Option<QString>& source);
    void setHeadingReference(const bmcl::Option<float>& heading);
    void setHeadingMagnetic(const bmcl::Option<float>& heading);
    void setHeadingSettings(const MotionIndicatorSettings& settings);

    void setVerticalSpeedMeasured(const bmcl::Option<float>& verticalSpeed);
    void setVerticalSpeedIndicatorLimit(float speedLimit);

    void setAngleOfAttack(const bmcl::Option<float>& aoa);

    void setWindDirection(const bmcl::Option<float>& dir);
    void setWindVelocity(const bmcl::Option<float>& vel);

    bmcl::Option<QString>   velocitySource() const;
    bmcl::Option<float>     velocityReference() const;
    bmcl::Option<QString>   velocityUnits() const;
    MotionIndicatorSettings velocitySettings() const;

    bmcl::Option<QString>   altitudeSource() const;
    bmcl::Option<float>     altitudeReference() const;
    bool                    showScale() const;
    MotionIndicatorSettings altitudeSettings() const;

    bmcl::Option<float>     rollReference() const;
    MotionIndicatorSettings rollSettings() const;

    bmcl::Option<QString>   headingSource() const;
    bmcl::Option<float>     headingReference() const;
    bmcl::Option<float>     headingMagnetic() const;
    MotionIndicatorSettings headingSettings() const;

    bmcl::Option<float>     verticalSpeedMeasured() const;
    float                   verticalSpeedIndicatorLimit() const;

    bmcl::Option<float>     angleOfAttack() const;

    bmcl::Option<float>     windDirection() const;
    bmcl::Option<float>     windVelocity() const;
signals:
    void velocityChanged();
    void altitudeChanged();
    void rollChanged();
    void angleOfAttackChanged();
    void windChanged();
    void headingUpdated();

private:
    bmcl::Option<QString>   _velocitySource;
    bmcl::Option<float>     _velocityReference;
    bmcl::Option<QString>   _velocityUnits;
    MotionIndicatorSettings _velocitySettings;

    bmcl::Option<QString>   _altitudeSource;
    bmcl::Option<float>     _altitudeReference;
    bool                    _showScale;
    MotionIndicatorSettings _altitudeSettings;

    bmcl::Option<QString>   _headingSource;
    bmcl::Option<float>     _headingReference;
    bmcl::Option<float>     _headingMagnetic;
    MotionIndicatorSettings _headingSettings;

    bmcl::Option<float>     _rollReference;
    MotionIndicatorSettings _rollSettings;

    bmcl::Option<float>     _verticalSpeedMeasured;
    float                   _verticalSpeedIndicatorLimit;

    bmcl::Option<float>     _angleOfAttack;

    bmcl::Option<float>     _windDirection;
    bmcl::Option<float>     _windVelocity;

    Q_DISABLE_COPY(MotionExtension)
};

}

