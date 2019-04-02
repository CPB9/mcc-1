moc_headers = [
  'ChannelsListDelegate.h',
  'ChannelsListModel.h',
  'ChannelsListView.h',
  'ChannelsPropertiesDialog.h',
  'ChannelsPropertiesWidget.h',
  'ChannelsWidget.h',
]

processed = qt5_mod.preprocess(
  moc_headers : moc_headers,
  qresources : 'toolbar-channels.qrc',
)

src = [
  'ChannelsListDelegate.cpp',
  'ChannelsListModel.cpp',
  'ChannelsListView.cpp',
  'ChannelsPlugin.cpp',
  'ChannelsPropertiesDialog.cpp',
  'ChannelsPropertiesWidget.cpp',
  'ChannelsWidget.cpp',
]

all_mcc_plugins += shared_library('plugin-mcc-toolbar-channels',
  name_prefix : '',
  sources : src + processed,
  link_with : [mcc_ide_lib],
  include_directories : mcc_inc,
  dependencies : [mcc_plugin_dep, mcc_msg_dep, mcc_res_dep, mcc_uav_dep],
)
