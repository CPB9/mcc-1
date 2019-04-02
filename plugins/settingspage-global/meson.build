ui_files = [
  'GlobalSettingsPage.ui',
]

moc_headers = [
  'GlobalSettingsPage.h',
]

src = [
  'GlobalSettingsPage.cpp',
]

processed = qt5_mod.preprocess(
  moc_headers : moc_headers,
  ui_files : ui_files,
  include_directories : mcc_inc,
)

all_mcc_plugins += shared_library('plugin-mcc-settingspage-global',
  name_prefix : '',
  sources : src + processed,
  include_directories : mcc_inc,
  dependencies : [mcc_plugin_dep, mcc_geo_dep, mcc_ui_dep, mcc_res_dep],
)
