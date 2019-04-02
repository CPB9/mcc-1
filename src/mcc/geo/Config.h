#pragma once

#ifdef BUILDING_MCC_GEO
# ifdef _MSC_VER
#  define MCC_GEO_DECLSPEC __declspec(dllexport)
# else
#  define MCC_GEO_DECLSPEC __attribute__ ((__visibility__ ("default")))
# endif
#else
# ifdef _MSC_VER
#  define MCC_GEO_DECLSPEC __declspec(dllimport)
# else
#  define MCC_GEO_DECLSPEC
# endif
#endif
