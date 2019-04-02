#pragma once

#include "mcc/vis/Config.h"

#include <QRgb>

#include <cstdint>
#include <string>

namespace mccvis {

struct ViewParams {
    ViewParams()
        : minBeamDistance(1000)
        , maxBeamDistance(60000)
        , minAzimuth(0)
        , maxAzimuth(360)
        , radarHeight(5)
        , minAngle(-5)
        , maxAngle(70)
        , deltaAngle(0.0001)
        , angleStep(1.0)
        , frequency(2800)
        , calcStep(90)
        , objectHeight(50)
        , targetSpeed(343)
        , missleSpeed(2000)
        , minHitDistance(3000)
        , maxHitDistance(60000)
        , reactionTime(2)
        , externReactionTime(2)
        , viewZonesColorArgb(qRgba(178, 223, 138, 0.5 * 255))
        , hitZonesColorArgb(qRgba(251, 154, 153, 0.5 * 255))
        , additionalDistancePercent(5)
        , isRelativeHeight(true)
        , isBidirectional(true)
        , useFresnelRegion(false)
        , useCalcStep(false)
        , isTargetRelativeHeight(true)
        , isTargetDirectedTowards(true)
        , calcHits(false)
        , canViewGround(true)
        , hasRefraction(false)
    {
    }

public:
    // 8
    double minBeamDistance;
    double maxBeamDistance;
    double minAzimuth;
    double maxAzimuth;
    double radarHeight;
    double minAngle;
    double maxAngle;
    double deltaAngle;
    double angleStep;
    double frequency;
    double calcStep;
    double objectHeight;
    double targetSpeed;
    double missleSpeed;
    double minHitDistance;
    double maxHitDistance;
    double reactionTime;
    double externReactionTime;
    // 4
    std::uint32_t viewZonesColorArgb;
    std::uint32_t hitZonesColorArgb;
    // 1
    std::uint8_t additionalDistancePercent;
    bool isRelativeHeight;
    bool isBidirectional;
    bool useFresnelRegion;
    bool useCalcStep;
    bool isTargetRelativeHeight;
    bool isTargetDirectedTowards;
    bool calcHits;
    bool canViewGround;
    bool hasRefraction;
    //var
    std::string name;
};
}
