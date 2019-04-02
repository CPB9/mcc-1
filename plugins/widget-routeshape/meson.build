moc_headers = [
  'CanvasPicker.h',
  'RouteSectionPlot.h',
  'RouteCurve.h'
]

src = [
  'CanvasPicker.cpp',
  'RouteSectionPlot.cpp',
  'RouteCurve.cpp',
  'RouteSectionWidgetPlugin.cpp',
]

processed = qt5_mod.preprocess(
  include_directories : mcc_inc,
  moc_headers : moc_headers,
  qresources : 'RouteShapeWidgetPlugin.qrc',
)

all_mcc_plugins += shared_library('plugin-mcc-widget-routeshape',
  name_prefix : '',
  sources : src + processed,
  link_with : [mcc_map_lib],
  include_directories : mcc_inc,
  dependencies : [qt5_core_dep, bmcl_dep, qt5_gui_dep, qwt_dep, qt5_widgets_dep, mcc_hm_dep, mcc_vis_dep, mcc_msg_dep, mcc_plugin_dep, mcc_uav_dep],
)
