#pragma once

#ifdef BUILDING_MCC_RES
# ifdef _MSC_VER
#  define MCC_RES_DECLSPEC __declspec(dllexport)
# else
#  define MCC_RES_DECLSPEC __attribute__ ((__visibility__ ("default")))
# endif
#else
# ifdef _MSC_VER
#  define MCC_RES_DECLSPEC __declspec(dllimport)
# else
#  define MCC_RES_DECLSPEC
# endif
#endif

