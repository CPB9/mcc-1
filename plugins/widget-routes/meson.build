headers = [
  'MissionPlanDelegate.h',
  'RouteEditorTool.h',
  'RouteListTreeView.h',
  'MissionPlanModel.h',
  'RouteListModel.h',
]

uis = [
  'uifiles/ScanAreaDialog.ui',
]

src = [
  'MissionPlanDelegate.cpp',
  'RouteEditorTool.cpp',
  'ScanAreaDialog.cpp',
  'TableViewWithFreezeSelection.cpp',
  'Plugin.cpp',
  'RouteListTreeView.cpp',
  'RouteListFilterModel.cpp',
  'RouteListModel.cpp',
  'MissionPlanModel.cpp',

]

processed = qt5_mod.preprocess(
  include_directories : mcc_inc,
  moc_headers : headers,
  ui_files : uis,
  qresources : 'widget-routes.qrc',
)

all_mcc_plugins += shared_library('plugin-mcc-widget-routes',
  name_prefix : '',
  sources : src + processed,
  link_with : [mcc_map_lib, mcc_ide_lib],
  include_directories : mcc_inc,
  dependencies : [bmcl_dep, fmt_dep, qt5_core_dep, qt5_gui_dep, qt5_widgets_dep, qwt_dep, mcc_hm_dep, mcc_msg_dep, mcc_plugin_dep, mcc_res_dep, mcc_uav_dep, mcc_ui_dep],
)
