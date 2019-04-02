moc_headers = [
  'CafService.h',
]

src = [
  'CafService.cpp',
  'CafPlugin.cpp',
]

processed = qt5_mod.preprocess(
  moc_headers : moc_headers,
  include_directories : mcc_inc,
)

all_mcc_plugins += shared_library('plugin-mcc-uiexch-net',
  name_prefix : '',
  sources : src + processed,
  link_with : [mcc_plugin_net_lib],
  include_directories : mcc_inc,
  dependencies : [mcc_plugin_dep, mcc_geo_dep, libcaf_core_dep, mcc_msg_dep, mcc_net_dep, mcc_error_dep, mcc_uav_dep],
)
