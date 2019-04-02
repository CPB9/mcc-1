moc_headers = [
  'EditUavDialog.h',
]

processed = qt5_mod.preprocess(
  include_directories : mcc_inc,
  moc_headers : moc_headers,
)

src = [
  'EditUavDialog.cpp',
  'EditUavDialogPlugin.cpp',
]

all_mcc_plugins += shared_library('plugin-mcc-dialog-uav-edit',
  name_prefix : '',
  sources : src + processed,
  link_with : [mcc_ide_lib, mcc_msg_lib],
  include_directories : mcc_inc,
  dependencies : [mcc_plugin_dep, mcc_geo_dep, qt5_core_dep, qt5_gui_dep, qt5_widgets_dep, mcc_res_dep, mcc_uav_dep],
)
