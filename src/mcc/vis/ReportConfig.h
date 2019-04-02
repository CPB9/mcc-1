#pragma once

#include "mcc/vis/Config.h"
#include "mcc/vis/ProfileViewer.h"
#include "mcc/vis/RegionViewer.h"

#include <cstddef>
#include <cstdint>

namespace mccvis {

struct ReportConfig {
    ReportConfig()
        : profileImageWidth(1920)
        , profileImageHeight(1080)
        , zoneImageWidth(1920)
        , zoneImageHeight(1080)
        , anglesImageWidth(1920)
        , anglesImageHeight(1080)
        , profileRenderCfg()
        , zoneRenderCfg()
        , anglesRenderCfg()
        , genExcelReport(true)
        , genProfileImages(true)
        , genZoneImages(true)
        , genAnglesImages(true)
        , profilesPerImage(2)
        , profilesPerExcelPage(2)
    {
    }
    unsigned profileImageWidth;
    unsigned profileImageHeight;
    unsigned zoneImageWidth;
    unsigned zoneImageHeight;
    unsigned anglesImageWidth;
    unsigned anglesImageHeight;
    ProfileViewer::RenderConfig profileRenderCfg;
    RegionViewer::RenderConfig zoneRenderCfg;
    RegionViewer::RenderConfig anglesRenderCfg;
    bool genExcelReport;
    bool genProfileImages;
    bool genZoneImages;
    bool genAnglesImages;
    std::uint8_t profilesPerImage;
    std::uint8_t profilesPerExcelPage;
};

}

