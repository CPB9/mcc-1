src = [
  'AddUavDialogPlugin.cpp',
]

all_mcc_plugins += shared_library('plugin-mcc-dialog-uav-add',
  name_prefix : '',
  sources : src,
  link_with : [mcc_ide_lib],
  include_directories : mcc_inc,
  dependencies : [mcc_plugin_dep, mcc_geo_dep, qt5_core_dep, qt5_gui_dep, qt5_widgets_dep, mcc_uav_dep],
)
