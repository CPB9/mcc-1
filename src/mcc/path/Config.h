#pragma once

#ifdef BUILDING_MCC_PATH
# ifdef _MSC_VER
#  define MCC_PATH_DECLSPEC __declspec(dllexport)
# else
#  define MCC_PATH_DECLSPEC __attribute__ ((__visibility__ ("default")))
# endif
#else
# ifdef _MSC_VER
#  define MCC_PATH_DECLSPEC __declspec(dllimport)
# else
#  define MCC_PATH_DECLSPEC
# endif
#endif

