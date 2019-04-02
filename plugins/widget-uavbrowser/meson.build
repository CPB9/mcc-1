src = [
  'UavBrowserPlugin.cpp'
]

all_mcc_plugins += shared_library('plugin-mcc-widget-uavbrowser',
  name_prefix : '',
  sources : src,
  link_with : [mcc_ide_lib],
  include_directories : mcc_inc,
  dependencies : [mcc_plugin_dep, mcc_geo_dep, mcc_uav_dep],
)
