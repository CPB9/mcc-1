moc_headers = [
  'ProfileDataViewer.h',
  'ProfileViewer.h',
  'RadarGroup.h',
  'RegionViewer.h',
  'ReportGen.h',
  'ResultsWidget.h',
]

src = [
  'Profile.cpp',
  'ProfileDataViewer.cpp',
  'ProfileViewer.cpp',
  'Radar.cpp',
  'RadarGroup.cpp',
  'Region.cpp',
  'RegionViewer.cpp',
  'ReportGen.cpp',
  'ResultsWidget.cpp',
  'Ticks.cpp',
]

processed = qt5_mod.preprocess(
    include_directories : mcc_inc,
    moc_headers : moc_headers,
)

private_deps = [thread_dep, xlnt_dep]
deps = [bmcl_dep, qt5_widgets_dep, qt5_printsupport_dep, mcc_geo_dep, mcc_hm_dep, mcc_plugin_dep, omp_dep, qt5_gui_dep]

mcc_vis_lib = shared_library('mcc-vis',
  name_prefix : 'lib',
  sources : src + processed,
  include_directories : mcc_inc,
  dependencies : deps + private_deps,
  cpp_args : '-DBUILDING_MCC_VIS',
)

mcc_vis_dep = declare_dependency(
  link_with : mcc_vis_lib,
  include_directories : mcc_inc,
  dependencies : deps,
)

all_mcc_libs += mcc_vis_lib
