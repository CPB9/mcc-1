rc_file = 'settingspage-gps.qrc'

ui_files = [
  'GpsSettingsPage.ui',
]

moc_headers = [
  'GpsSettingsPage.h',
]

src = [
  'GpsSettingsPage.cpp',
]

processed = qt5_mod.preprocess(
  moc_headers : moc_headers,
  qresources : rc_file,
  ui_files : ui_files,
  include_directories : mcc_inc,
)

all_mcc_plugins += shared_library('plugin-mcc-settingspage-gps',
  name_prefix : '',
  sources : src + processed,
  link_with : [mcc_ide_lib],
  include_directories : mcc_inc,
  dependencies : [mcc_plugin_dep, mcc_geo_dep, mcc_ui_dep, qt5_gui_dep],
)
