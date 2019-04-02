#include "mcc/ui/Gradient.h"

namespace mccui {

static const std::array<Rgb<uint8_t>, 14> spectralTheme = {
    //Spectral
    Rgb<uint8_t>{ 94,  79, 162},
    Rgb<uint8_t>{ 50, 136, 189},
    Rgb<uint8_t>{102, 194, 165},
    Rgb<uint8_t>{171, 221, 164},
    Rgb<uint8_t>{230, 245, 152},
    Rgb<uint8_t>{255, 255, 191},
    Rgb<uint8_t>{254, 224, 139},
    Rgb<uint8_t>{253, 174,  97},
    Rgb<uint8_t>{244, 109,  67},
    Rgb<uint8_t>{213,  62,  79},
    Rgb<uint8_t>{158,   1,  66},

    //RdYlGn
    //{  0, 104,  55},
    //{ 26, 152,  80},
    //{102, 189,  99},
    //{166, 217, 106},
    //{217, 239, 139},
    //{255, 255, 191},
    //{254, 224, 139},
    //{253, 174,  97},
    //{244, 109,  67},
    //{215,  48,  39},
    //{165,   0,  38},
    // greys
    Rgb<uint8_t>{200, 200, 200},
    Rgb<uint8_t>{225, 225, 225},
    Rgb<uint8_t>{255, 255, 255},
};

static const MapGradient spectralGradient(0, 3500, spectralTheme);

MCC_UI_DECLSPEC const MapGradient& getSpectralMapGradient()
{
    return spectralGradient;
}
}
