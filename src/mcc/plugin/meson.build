src = [
  'Plugin.cpp',
  'PluginCache.cpp',
  'PluginData.cpp',
]

deps = [bmcl_dep]

mcc_plugin_lib = shared_library('mcc-plugin',
  name_prefix : 'lib',
  sources : src,
  dependencies : deps,
  cpp_args : '-DBUILDING_MCC_PLUGIN',
  include_directories : mcc_inc,
)

mcc_plugin_dep = declare_dependency(
  link_with : mcc_plugin_lib,
  include_directories : mcc_inc,
  dependencies : deps,
)

all_mcc_libs += mcc_plugin_lib
