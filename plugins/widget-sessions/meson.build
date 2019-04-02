rc_file = 'toolbar-sessions.qrc'

moc_headers = [
    'SessionsWidget.h',
    'SessionsModel.h'
]

ui_files = []

src = [
  'Plugin.cpp',
  'SessionsWidget.cpp',
  'SessionsModel.cpp'
]

processed = qt5_mod.preprocess(
  qresources : rc_file,
  ui_files : ui_files,
  moc_headers : moc_headers,
  include_directories : mcc_inc,
)

all_mcc_plugins += shared_library('plugin-mcc-toolbar-sessions',
  name_prefix : '',
  sources : src + processed,
  link_with : [],
  include_directories : mcc_inc,
  dependencies : [mcc_plugin_dep, mcc_msg_dep, mcc_uav_dep],
)
