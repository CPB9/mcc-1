moc_headers = [
  'CoordinateSystemPropertiesDialog.h',
  'CoordinateSystemWidget.h',
  'CoordinateSystemSettingsPage.h',
  'CoordinateConverterTool.h',
]

processed = qt5_mod.preprocess(
  moc_headers : moc_headers,
  qresources : 'toolbar-coordinatesystem.qrc',
)

src = [
  'CoordinateSystemPropertiesDialog.cpp',
  'CoordinateSystemWidget.cpp',
  'CoordinateSystemPlugin.cpp',
  'CoordinateSystemSettingsPage.cpp',
  'CoordinateConverterTool.cpp',
]

all_mcc_plugins += shared_library('plugin-mcc-toolbar-coordinatesystem',
  name_prefix : '',
  sources : src + processed,
  link_with : [mcc_ide_lib],
  include_directories : mcc_inc,
  dependencies : [mcc_plugin_dep, mcc_uav_dep, mcc_geo_dep, mcc_res_dep],
)
