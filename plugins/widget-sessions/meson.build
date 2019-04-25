rc_file = 'toolbar-sessions.qrc'

moc_headers = [
    'SessionsWidget.h',
    'SessionsModel.h',
    'SessionTool.h',
    'DeviceItem.h',
    'DeviceListView.h',
    'ScreenRecorder.h',
    'VideoRecordWidget.h',
    'SettingsPage.h'
]

ui_files = []

src = [
  'Plugin.cpp',
  'SessionsWidget.cpp',
  'SessionsModel.cpp',
  'SessionTool.cpp',
  'DeviceItem.cpp',
  'DeviceListView.cpp',
  'ScreenRecorder.cpp',
  'VideoRecordWidget.cpp',
  'SettingsPage.cpp'
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
  link_with : [mcc_ide_lib],
  include_directories : mcc_inc,
  dependencies : [mcc_plugin_dep, mcc_msg_dep, mcc_uav_dep, mcc_path_dep, fmt_dep, mcc_res_dep],
)
