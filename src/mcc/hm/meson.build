src = [
  'SrtmReader.cpp',
  'Gl1File.cpp',
  'Gl30AllFile.cpp',
  'SrtmFileCache.cpp',
  'HmReader.cpp',
  'HmStackReader.cpp',
  'OmhmReader.cpp',
]

headers = [
  'SrtmReader.h',
  'Gl1File.h',
  'Gl30AllFile.h',
  'SrtmFileCache.h',
  'HmReader.h',
  'HmStackReader.h',
  'OmhmReader.h',
]

deps = [bmcl_dep, qt5_core_dep, mcc_geo_dep]

mcc_hm_lib = shared_library('mcc-hm',
  name_prefix : 'lib',
  sources : src,
  include_directories : mcc_inc,
  dependencies : deps + [lz4_dep],
  cpp_args : '-DBUILDING_MCC_HM',
)

mcc_hm_dep = declare_dependency(
  link_with : mcc_hm_lib,
  include_directories : mcc_inc,
  dependencies : deps,
)

all_mcc_libs += mcc_hm_lib

if gdal_dep.found()
  all_mcc_tools += executable('mcc-gdal-to-omhm',
    sources : 'gdal_to_omhm.cpp',
    dependencies : [gdal_dep, qt5_widgets_dep, bmcl_dep, mcc_hm_dep],
  )
endif
