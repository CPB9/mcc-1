#pragma once

#ifdef BUILDING_MCC_CRASHDUMP
# ifdef _MSC_VER
#  define MCC_CRASHDUMP_DECLSPEC __declspec(dllexport)
# else
#  define MCC_CRASHDUMP_DECLSPEC __attribute__ ((__visibility__ ("default")))
# endif
#else
# ifdef _MSC_VER
#  define MCC_CRASHDUMP_DECLSPEC __declspec(dllimport)
# else
#  define MCC_CRASHDUMP_DECLSPEC
# endif
#endif
