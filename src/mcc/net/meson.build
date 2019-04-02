src_error = [
  'Error.h',
  'Error.cpp',
]

mcc_error_lib = shared_library('mcc-error',
  name_prefix : 'lib',
  sources : src_error,
  dependencies : [fmt_dep, bmcl_dep, libcaf_core_dep, mcc_msg_dep],
  include_directories : mcc_inc,
  cpp_args : '-DBUILDING_MCC_ERROR',
)

mcc_error_dep = declare_dependency(link_with: mcc_error_lib,
  include_directories: mcc_inc,
)

all_mcc_libs += mcc_error_lib

src = [
  'channels/ChannelImpl.h',
  'channels/ChannelImpl.cpp',
  'channels/ChannelCom.h',
  'channels/ChannelCom.cpp',
  'channels/ChannelTcp.h',
  'channels/ChannelTcp.cpp',
  'channels/ChannelUdp.h',
  'channels/ChannelUdp.cpp',
  'channels/ChannelInvalid.h',
  'channels/ChannelInvalid.cpp',
  'Asio.h',
  'Asio.cpp',
  'Cmd.h',
  'Cmd.cpp',
  'Exchanger.h',
  'Exchanger.cpp',
  'Group.h',
  'Group.cpp',
  'TmHelper.h',
  'TmHelper.cpp',
  'NetName.h',
  'NetName.cpp',
  'NetLoggerInf.h',
  'NetLoggerInf.cpp',
  'Timer.h',
  'Timer.cpp',
  'NetPlugin.h',
  'NetPlugin.cpp',
]

mcc_plugin_net_lib = shared_library('mcc-plugin-net',
  name_prefix : 'lib',
  sources : src,
  include_directories : mcc_inc,
  dependencies : [bmcl_dep, mcc_msg_dep, asio_dep, fmt_dep, qt5_network_dep, libcaf_core_dep, mcc_msg_dep, mcc_error_dep, mcc_plugin_dep, mcc_path_dep],
  cpp_args : '-DBUILDING_MCC_PLUGIN_NET',
)

mcc_plugin_net_dep = declare_dependency(link_with: mcc_plugin_net_lib,
  include_directories: mcc_inc,
  dependencies: mcc_msg_dep,
)

all_mcc_libs += mcc_plugin_net_lib

subdir('db')

net_src = [
  'NetLoader.h',
  'NetLoader.cpp',
  'NetProxy.h',
  'NetProxy.cpp',
  'NetLogger.h',
  'NetLogger.cpp',
]

mcc_net_lib = shared_library('mcc-net',
  name_prefix : 'lib',
  sources : net_src,
  #extra_files : ['meson.build'],
  link_with : [mcc_plugin_net_lib, mcc_db_lib],
  include_directories : mcc_inc,
  dependencies : [bmcl_dep, mcc_msg_dep, mcc_error_dep, fmt_dep, mcc_msg_dep, mcc_plugin_dep, mcc_path_dep, libcaf_core_dep],
  cpp_args : '-DBUILDING_MCC_NET',
)

mcc_net_dep = declare_dependency(link_with: mcc_net_lib,
  include_directories: mcc_inc,
  dependencies: mcc_msg_dep
)

all_mcc_libs += mcc_net_lib

