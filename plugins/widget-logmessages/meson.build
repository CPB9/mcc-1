rc_file = 'logmessages.qrc'

moc_headers = [
  'EventViewer.h',
  'MessageListTool.h',
]

src = [
  'EventViewer.cpp',
  'Plugin.cpp',
  'MessageListTool.cpp',
]

processed = qt5_mod.preprocess(
  include_directories : mcc_inc,
  qresources : rc_file,
  moc_headers : moc_headers,
)

all_mcc_plugins += shared_library('plugin-mcc-widget-logmessages',
  name_prefix : '',
  sources : src + processed,
  link_with : [mcc_ide_lib],
  include_directories : mcc_inc,
  dependencies : [bmcl_dep, qt5_core_dep, qt5_gui_dep, qt5_widgets_dep, mcc_plugin_dep, mcc_geo_dep, mcc_uav_dep],
)
