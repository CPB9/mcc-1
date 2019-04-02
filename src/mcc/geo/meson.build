src = [
  'CoordinateConverter.cpp',
  'EnuPositionHandler.cpp',
  'LatLon.cpp',
  'LocalSystem.cpp',
  'MercatorProjection.cpp',
  'Attitude.cpp',
  'Position.cpp',
  'Vector3D.cpp',
  'GroupGeometry.cpp',
  'Bbox.cpp',
  'detail/geodesic.cpp',
]

headers = [
  'CoordinateConverter.h',
  'EnuPositionHandler.h',
  'LatLon.h',
  'LocalSystem.h',
  'MercatorProjection.h',
  'Attitude.h',
  'Position.h',
  'Vector3D.h',
  'GroupGeometry.h',
  'Bbox.h',
  'Geod.h',
  'Point.h',
]

deps = [bmcl_dep, qt5_core_dep]

mcc_geo_lib = shared_library('mcc-geo',
  name_prefix : 'lib',
  sources : src + headers,
  include_directories : mcc_inc,
  dependencies : deps + [proj_dep],
  cpp_args : '-DBUILDING_MCC_GEO',
)

mcc_geo_dep = declare_dependency(
  link_with : mcc_geo_lib,
  include_directories : mcc_inc,
  dependencies : deps,
)

all_mcc_libs += mcc_geo_lib
