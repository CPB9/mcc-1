#pragma once

#include "mcc/DeployConfig.h"

#define MCC_INIT_QRESOURCES(name)                 \
                                                  \
extern int qInitResources_##name();               \
                                                  \
class MccInitializer##name {                      \
public:                                           \
    MccInitializer##name()                        \
    {                                             \
        qInitResources_##name();                  \
    }                                             \
};                                                \
                                                  \
static MccInitializer##name mccInitializer##name;


#ifdef _MSC_VER
# define MCC_DECL_EXPORT __declspec(dllexport)
# define MCC_DECL_IMPORT __declspec(dllimport)
#else
# define MCC_DECL_EXPORT __attribute__ ((__visibility__ ("default")))
# define MCC_DECL_IMPORT
#endif

#ifdef BUILDING_MCC_ERROR
# define MCC_ERROR_DECLSPEC MCC_DECL_EXPORT
#else
# define MCC_ERROR_DECLSPEC MCC_DECL_IMPORT
#endif

#ifdef BUILDING_MCC_UI
# define MCC_UI_DECLSPEC MCC_DECL_EXPORT
#else
# define MCC_UI_DECLSPEC MCC_DECL_IMPORT
#endif

#ifdef BUILDING_MCC_UAV
# define MCC_UAV_DECLSPEC MCC_DECL_EXPORT
#else
# define MCC_UAV_DECLSPEC MCC_DECL_IMPORT
#endif

#ifdef BUILDING_MCC_MAP
# define MCC_MAP_DECLSPEC MCC_DECL_EXPORT
#else
# define MCC_MAP_DECLSPEC MCC_DECL_IMPORT
#endif

#ifdef BUILDING_MCC_DB
# define MCC_DB_DECLSPEC MCC_DECL_EXPORT
#else
# define MCC_DB_DECLSPEC MCC_DECL_IMPORT
#endif

#ifdef BUILDING_MCC_IDE
# define MCC_IDE_DECLSPEC MCC_DECL_EXPORT
#else
# define MCC_IDE_DECLSPEC MCC_DECL_IMPORT
#endif

#ifdef BUILDING_MCC_MSG
# define MCC_MSG_DECLSPEC MCC_DECL_EXPORT
#else
# define MCC_MSG_DECLSPEC MCC_DECL_IMPORT
#endif

#ifdef BUILDING_MCC_3D
# define MCC_3D_DECLSPEC MCC_DECL_EXPORT
#else
# define MCC_3D_DECLSPEC MCC_DECL_IMPORT
#endif

#ifdef BUILDING_MCC_PLUGIN_NET
# define MCC_PLUGIN_NET_DECLSPEC MCC_DECL_EXPORT
#else
# define MCC_PLUGIN_NET_DECLSPEC MCC_DECL_IMPORT
#endif

#ifdef BUILDING_MCC_CORE
# define MCC_CORE_DECLSPEC MCC_DECL_EXPORT
#else
# define MCC_CORE_DECLSPEC MCC_DECL_IMPORT
#endif

#ifdef BUILDING_MCC_NET
# define MCC_NET_DECLSPEC MCC_DECL_EXPORT
#else
# define MCC_NET_DECLSPEC MCC_DECL_IMPORT
#endif

#ifdef BUILDING_MCC_CALIB
# define MCC_CALIB_DECLSPEC MCC_DECL_EXPORT
#else
# define MCC_CALIB_DECLSPEC MCC_DECL_IMPORT
#endif

#ifdef BUILDING_MCC_QML
# define MCC_QML_DECLSPEC MCC_DECL_EXPORT
#else
# define MCC_QML_DECLSPEC MCC_DECL_IMPORT
#endif

#define MCC_DELETE_MOVE_CONSTRUCTORS(Name)      \
  Name(Name&&) = delete;                        \
  void operator=(Name&&) = delete

#define MCC_DELETE_COPY_CONSTRUCTORS(Name)      \
  Name(const Name&) = delete;                   \
  void operator=(const Name&) = delete

#define MCC_DELETE_COPY_MOVE_CONSTRUCTORS(Name) \
    MCC_DELETE_COPY_CONSTRUCTORS(Name);         \
    MCC_DELETE_MOVE_CONSTRUCTORS(Name);
