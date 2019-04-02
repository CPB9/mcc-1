#pragma once
#include "mcc/Config.h"
#include <vector>
#include <bitset>
#include <array>
#include <bmcl/Option.h>
#include "mcc/msg/Msg.h"
#include "mcc/msg/Tm.h"
#include "mcc/msg/Cmd.h"


namespace mccmsg {

struct MCC_MSG_DECLSPEC CalibrationSideStatus
{
    bool done;
    bool visible;
    bool inProgress;
    bool rotate;

    void reset()
    {
        done = false;
        visible = false;
        inProgress = false;
        rotate = false;
    }
};

struct MCC_MSG_DECLSPEC CalibrationFlightModes
{
    bmcl::Option<int> killSwitchChannel;
    bmcl::Option<int> offboardSwitchChannel;
    bmcl::Option<int> returnSwitchChannel;
    bmcl::Option<int> flightModeChannel;

    bmcl::Option<int> mode1;
    bmcl::Option<int> mode2;
    bmcl::Option<int> mode3;
    bmcl::Option<int> mode4;
    bmcl::Option<int> mode5;
    bmcl::Option<int> mode6;
};

struct MCC_MSG_DECLSPEC CalibrationChannelLimits
{
    int rev;
    int min;
    int max;
    int trim;
    int dz;

    CalibrationChannelLimits()
    {
        rev = 0;
        min = 0;
        max = 0;
        trim = 0;
        dz = 0;
    }
};

enum class CalibrationCmd : uint8_t
{
    NextStep,
    SkipStep,
    SetFlightModes
};

enum class CalibrationSensor : uint8_t
{
    Accelerometer,
    Magnetometer,
    Gyroscope,
    Level,
    Esc,
    Radio,
    FlightModes
};

class MCC_MSG_DECLSPEC Calibration
{
public:
    static constexpr int MAX_RC_CHANNELS = 18;

    inline Calibration() { reset(); }
    CalibrationSideStatus calDownSide;
    CalibrationSideStatus calUpsideDownSide;
    CalibrationSideStatus calLeftSide;
    CalibrationSideStatus calRightSide;
    CalibrationSideStatus calNoseDownSide;
    CalibrationSideStatus calTailDownSide;

    void reset()
    {
        calDownSide.reset();
        calUpsideDownSide.reset();
        calLeftSide.reset();
        calRightSide.reset();
        calNoseDownSide.reset();
        calTailDownSide.reset();

        waitingForCancel = false;
        failed = false;
        done = false;

        progress = 0;
        nextEnabled = false;
        skipEnabled = false;
    }

    CalibrationSensor _sensor;

    bool waitingForCancel;
    bool failed;
    bool done;

    size_t progress;
    std::string message;

    std::array<bmcl::Option<uint16_t>, MAX_RC_CHANNELS> pwmValues;
    std::string image;
    bool nextEnabled;
    bool skipEnabled;

    bool isMagnetometer() const { return _sensor == CalibrationSensor::Magnetometer; }

    CalibrationFlightModes flightModes;
    CalibrationChannelLimits channelLimits;
};

class MCC_MSG_DECLSPEC TmCalibration : public TmAny
{
public:
    inline TmCalibration(const Device& device, const Calibration& state) : TmAny(device), _state(state){}
    void visit(TmVisitor* visitor) const override;
    const Calibration& state() const { return _state; }
private:
    Calibration _state;
};

class MCC_MSG_DECLSPEC TmCommonCalibrationStatus : public TmAny
{
public:
    inline TmCommonCalibrationStatus(const Device& device) : TmAny(device)
    {
        accelerometer = false;
        magnetometer = false;
        gyroscope = false;
        level = false;
        esc = false;
        radio = false;
    }

    bool isSensorCalibrated(CalibrationSensor sensor) const
    {
        switch (sensor)
        {
            case CalibrationSensor::Accelerometer: return accelerometer;
            case CalibrationSensor::Magnetometer: return magnetometer;
            case CalibrationSensor::Gyroscope: return gyroscope;
            case CalibrationSensor::Level: return level;
            case CalibrationSensor::Esc: return esc;
            case CalibrationSensor::Radio: return radio;
            case CalibrationSensor::FlightModes: return true;
            default:
                break;
        }
        return false;
    }
    void visit(TmVisitor* visitor) const override;

    bool accelerometer;
    bool magnetometer;
    bool gyroscope;
    bool level;
    bool esc;
    bool radio;
};

class MCC_MSG_DECLSPEC CmdCalibrationStart : public DevReq
{
public:
    CmdCalibrationStart(const Device& device, CalibrationSensor component,
        const bmcl::Option<CalibrationCmd>& specialCmd = bmcl::None,
        const bmcl::Option<CalibrationFlightModes>& flightModes = bmcl::None);
    ~CmdCalibrationStart();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
    CalibrationSensor component() const;
    const bmcl::Option<CalibrationCmd>& specialCmd() const;
    const bmcl::Option<CalibrationFlightModes>& flightModes() const;
private:
    CalibrationSensor _component;
    bmcl::Option<CalibrationCmd> _specialCmd;
    bmcl::Option<CalibrationFlightModes> _flightModes;
};

class MCC_MSG_DECLSPEC CmdCalibrationCancel : public DevReq
{
public:
    CmdCalibrationCancel(const Device& device);
    ~CmdCalibrationCancel();
    void visit(CmdVisitor* visitor) const override;
    const char* nameXXX() const override;
    const char* info() const override;
};

}
