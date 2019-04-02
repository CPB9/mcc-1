#include "mcc/uav/MotionExtension.h"

#include <bmcl/DoubleEq.h>

namespace mccuav
{

bool operator==(const bmcl::Option<float>& left, const bmcl::Option<float>& right)
{
    if (left.isSome() && right.isSome()) {
        return bmcl::doubleEq(left.unwrap(), right.unwrap());
    }
    return left.isSome() == right.isSome();
}

MotionExtension::MotionExtension(QObject* parent)
{
    _verticalSpeedIndicatorLimit = 20;
}

void MotionExtension::setVelocitySource(const bmcl::Option<QString>& source)
{
    if (_velocitySource == source)
        return;
    _velocitySource = source;
    emit velocityChanged();
}

void MotionExtension::setVelocityReference(const bmcl::Option<float>& velocity)
{
    if(_velocityReference == velocity)
        return;
    _velocityReference = velocity;
    emit velocityChanged();
}

void MotionExtension::setVelocityUnits(const bmcl::Option<QString>& units)
{
    _velocityUnits = units;
    emit velocityChanged();
}

void MotionExtension::setVelocitySettings(const MotionIndicatorSettings& settings)
{
    _velocitySettings = settings;
    emit velocityChanged();
}

void MotionExtension::setAltitudeSource(const bmcl::Option<QString>& source)
{
    if (_altitudeSource == source)
        return;
    _altitudeSource = source;
    emit altitudeChanged();
}

void MotionExtension::setAltitudeReference(bmcl::Option<float> alt)
{
    if (alt == _altitudeReference)
        return;
    _altitudeReference = alt;
    emit altitudeChanged();
}

void MotionExtension::setShowScale(bool showScale)
{
    _showScale = showScale;
    emit altitudeSource();
}

void MotionExtension::setAltitudeSettings(const MotionIndicatorSettings& settings)
{
    _altitudeSettings = settings;
    emit altitudeSettings();
}

void MotionExtension::setRollReference(const bmcl::Option<float>& roll)
{
    if (_rollReference == roll)
        return;
    _rollReference = roll;
    emit rollChanged();
}

void MotionExtension::setHeadingSource(const bmcl::Option<QString>& source)
{
    if (_headingSource == source)
        return;
    _headingSource = source;
    emit headingUpdated();
}

void MotionExtension::setHeadingReference(const bmcl::Option<float>& heading)
{
    if (_headingReference == heading)
        return;
    _headingReference = heading;
    emit headingUpdated();
}

void MotionExtension::setHeadingMagnetic(const bmcl::Option<float>& heading)
{
    if (_headingMagnetic == heading)
        return;
    _headingMagnetic = heading;
    emit headingUpdated();
}

void MotionExtension::setHeadingSettings(const MotionIndicatorSettings& settings)
{
    _headingSettings = settings;
    emit headingUpdated();
}

void MotionExtension::setVerticalSpeedMeasured(const bmcl::Option<float>& verticalSpeed)
{
    if (_verticalSpeedMeasured == verticalSpeed)
        return;
    _verticalSpeedMeasured = verticalSpeed;
    emit altitudeChanged();
}

void MotionExtension::setVerticalSpeedIndicatorLimit(float speedLimit)
{
    _verticalSpeedIndicatorLimit = speedLimit;
    emit altitudeChanged();
}

void MotionExtension::setAngleOfAttack(const bmcl::Option<float>& aoa)
{
    if (_angleOfAttack == aoa)
        return;
    _angleOfAttack = aoa;
    emit angleOfAttackChanged();
}

void MotionExtension::setWindDirection(const bmcl::Option<float>& dir)
{
    if (_windDirection == dir)
        return;
    _windDirection = dir;
    emit windChanged();
}

void MotionExtension::setWindVelocity(const bmcl::Option<float>& vel)
{
    if (_windVelocity == vel)
        return;
    _windVelocity = vel;
    emit windChanged();
}

bmcl::Option<QString>           MotionExtension::velocitySource() const                 { return _velocitySource;               }
bmcl::Option<float>             MotionExtension::velocityReference() const              { return _velocityReference;            }
bmcl::Option<QString>           MotionExtension::velocityUnits() const                  { return _velocityUnits;                }
mccuav::MotionIndicatorSettings MotionExtension::velocitySettings() const               { return _velocitySettings;             }
bmcl::Option<QString>           MotionExtension::altitudeSource() const                 { return _altitudeSource;               }
bmcl::Option<float>             MotionExtension::altitudeReference() const              { return _altitudeReference;            }
bool                            MotionExtension::showScale() const                      { return _showScale;                    }
mccuav::MotionIndicatorSettings MotionExtension::altitudeSettings() const               { return _altitudeSettings;             }
bmcl::Option<float>             MotionExtension::rollReference() const                  { return _rollReference;                }
mccuav::MotionIndicatorSettings MotionExtension::rollSettings() const                   { return _rollSettings;                 }
bmcl::Option<QString>           MotionExtension::headingSource() const                  { return _headingSource;                }
bmcl::Option<float>             MotionExtension::headingReference() const               { return _headingReference;             }
bmcl::Option<float>             MotionExtension::headingMagnetic() const                { return _headingMagnetic;              }
mccuav::MotionIndicatorSettings MotionExtension::headingSettings() const                { return _headingSettings;              }
bmcl::Option<float>             MotionExtension::verticalSpeedMeasured() const          { return _verticalSpeedMeasured;        }
float                           MotionExtension::verticalSpeedIndicatorLimit() const    { return _verticalSpeedIndicatorLimit;  }
bmcl::Option<float>             MotionExtension::angleOfAttack() const                  { return _angleOfAttack;                }
bmcl::Option<float>             MotionExtension::windDirection() const                  { return _windDirection;                }
bmcl::Option<float>             MotionExtension::windVelocity() const                   { return _windVelocity;                 }

}

