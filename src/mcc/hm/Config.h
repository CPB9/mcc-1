#pragma once

#ifdef BUILDING_MCC_HM
# ifdef _MSC_VER
#  define MCC_HM_DECLSPEC __declspec(dllexport)
# else
#  define MCC_HM_DECLSPEC __attribute__ ((__visibility__ ("default")))
# endif
#else
# ifdef _MSC_VER
#  define MCC_HM_DECLSPEC __declspec(dllimport)
# else
#  define MCC_HM_DECLSPEC
# endif
#endif
