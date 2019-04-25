#include "mcc/ui/Gradient.h"

namespace mccui {

static const std::array<Rgb8, 12> spectralTheme = {
    //Spectral
    Rgb8{ 94,  79, 162},
    Rgb8{ 50, 136, 189},
    Rgb8{102, 194, 165},
    Rgb8{171, 221, 164},
    Rgb8{230, 245, 152},
    Rgb8{255, 255, 191},
    Rgb8{254, 224, 139},
    Rgb8{253, 174,  97},
    Rgb8{244, 109,  67},
    Rgb8{213,  62,  79},
    Rgb8{158,   1,  66},
    Rgb8{255, 255, 255},
};

// static const std::array<Rgb8, 12> rdYlGnTheme = {
//     Rgb8{  0, 104,  55},
//     Rgb8{ 26, 152,  80},
//     Rgb8{102, 189,  99},
//     Rgb8{166, 217, 106},
//     Rgb8{217, 239, 139},
//     Rgb8{255, 255, 191},
//     Rgb8{254, 224, 139},
//     Rgb8{253, 174,  97},
//     Rgb8{244, 109,  67},
//     Rgb8{215,  48,  39},
//     Rgb8{165,   0,  38},
//     Rgb8{255, 255, 255},
// };

// static const MapGradient rdYlGnGradient(0, 3000, rdYlGnGradient);

static const MapGradient spectralGradient(0, 3000, spectralTheme);

MCC_UI_DECLSPEC const MapGradient& getSpectralMapGradient()
{
    return spectralGradient;
}
}
