#pragma once

#ifdef BUILDING_MCC_VIS
# ifdef _MSC_VER
#  define MCC_VIS_DECLSPEC __declspec(dllexport)
# else
#  define MCC_VIS_DECLSPEC __attribute__ ((__visibility__ ("default")))
# endif
#else
# ifdef _MSC_VER
#  define MCC_VIS_DECLSPEC __declspec(dllimport)
# else
#  define MCC_VIS_DECLSPEC
# endif
#endif
