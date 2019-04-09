#ifndef KML_CONFIG_H__
#define KML_CONFIG_H__

#ifdef _MSC_VER
#define KML_DECL_DLLEXPORT __declspec(dllexport)
#define KML_DECL_DLLIMPORT __declspec(dllimport)
#else
#define KML_DECL_DLLEXPORT
#define KML_DECL_DLLIMPORT
#endif

#ifdef BUILDING_LIBKML
#define KML_EXPORT KML_DECL_DLLEXPORT
#else
#define KML_EXPORT KML_DECL_DLLIMPORT
#endif

#endif
