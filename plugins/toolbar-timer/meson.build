moc_headers = [
  'StopwatchTimerBaseWidget.h',
  'StopwatchTimerStatusWidget.h',
  'StopwatchTimerTool.h',
]

src = [
  'StopwatchTimerBaseWidget.cpp',
  'StopwatchTimerStatusWidget.cpp',
  'StopwatchTimerTool.cpp',
  'Plugin.cpp',
]

processed = qt5_mod.preprocess(
  include_directories : mcc_inc,
  moc_headers : moc_headers,
  qresources : 'Plugin.qrc',
)

all_mcc_plugins += shared_library('plugin-mcc-toolbar-timer',
  name_prefix : '',
  sources : src + processed,
  include_directories : mcc_inc,
  link_with : [mcc_ide_lib],
  dependencies : [bmcl_dep, qt5_core_dep, qt5_gui_dep, qt5_widgets_dep, mcc_uav_dep, mcc_plugin_dep, mcc_res_dep],
)
