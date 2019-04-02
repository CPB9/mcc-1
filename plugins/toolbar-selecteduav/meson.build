rc_file = 'toolbar-selecteduav.qrc'

moc_headers = [
  'AbstractUavWidget.h',
  'SelectedUavWidget.h',
  'ToolbarUavSettingsPage.h',
  'UavFailureWidget.h',
  'UavModeWidget.h',
  'UavNameWidget.h',
  'UavStatisticsWidget.h',
  'UavWidget.h',
  'UavsDialog.h',
]

processed = qt5_mod.preprocess(
  moc_headers : moc_headers,
  qresources : rc_file,
)

src = [
  'AbstractUavWidget.cpp',
  'SelectedUavPlugin.cpp',
  'SelectedUavWidget.cpp',
  'ToolbarUavSettingsPage.cpp',
  'UavFailureWidget.cpp',
  'UavModeWidget.cpp',
  'UavNameWidget.cpp',
  'UavStatisticsWidget.cpp',
  'UavWidget.cpp',
  'UavsDialog.cpp',
]

all_mcc_plugins += shared_library('plugin-mcc-toolbar-selecteduav',
  name_prefix : '',
  sources : src + processed,
  link_with : [mcc_ide_lib],
  include_directories : mcc_inc,
  dependencies : [mcc_plugin_dep, mcc_geo_dep, mcc_msg_dep, mcc_res_dep, mcc_uav_dep],
)
