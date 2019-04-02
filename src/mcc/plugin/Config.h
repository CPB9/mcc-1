#pragma once

#ifdef BUILDING_MCC_PLUGIN
# ifdef _MSC_VER
#  define MCC_PLUGIN_DECLSPEC __declspec(dllexport)
# else
#  define MCC_PLUGIN_DECLSPEC __attribute__ ((__visibility__ ("default")))
# endif
#else
# ifdef _MSC_VER
#  define MCC_PLUGIN_DECLSPEC __declspec(dllimport)
# else
#  define MCC_PLUGIN_DECLSPEC
# endif
#endif
