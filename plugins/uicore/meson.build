src = [
  'UiCorePlugin.cpp',
]

all_mcc_plugins += shared_library('plugin-mcc-uicore',
  name_prefix : '',
  sources : src,
  link_with : [],
  include_directories : mcc_inc,
  dependencies : [mcc_plugin_dep, mcc_hm_dep, mcc_uav_dep, mcc_msg_dep],
)
