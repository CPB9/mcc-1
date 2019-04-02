headers = [
    'OpenQmlFileDialog.h'
]

plugin_src = [
    'QmlPlugin.cpp',
    'OpenQmlFileDialog.cpp',
]

processed = qt5_mod.preprocess(
  include_directories : mcc_inc,
  moc_headers : headers,
)

plugin_mcc_qml = shared_library('plugin-mcc-qml',
  name_prefix : '',
  sources : plugin_src + processed,
  link_with : [mcc_qml_lib],
  include_directories : mcc_inc,
  dependencies : [qt5_core_dep, qt5_widgets_dep, mcc_plugin_dep, mcc_geo_dep, mcc_msg_dep, mcc_uav_dep],
)

all_mcc_plugins += plugin_mcc_qml


