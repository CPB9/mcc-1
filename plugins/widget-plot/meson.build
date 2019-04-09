moc_headers = [
    'PlotCurve.h',
    'PlotWidget.h',
    'PlotTool.h'
]

src = [
  'PlotWidgetPlugin.cpp',
  'PlotCurve.cpp',
  'PlotWidget.cpp',
  'PlotTool.cpp',
]

processed = qt5_mod.preprocess(
  moc_headers : moc_headers,
  include_directories : mcc_inc,
)

all_mcc_plugins += shared_library('plugin-mcc-widget-plot',
  name_prefix : '',
  sources : src + processed,
  link_with : [mcc_ide_lib],
  include_directories : mcc_inc,
  dependencies : [mcc_plugin_dep, mcc_geo_dep, mcc_uav_dep, qwt_dep, fmt_dep, mcc_msg_dep],
)
