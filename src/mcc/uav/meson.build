headers = [
  'ChannelsController.h',
  'ExchangeService.h',
  'GroupsController.h',
  'MotionExtension.h',
  'PointOfInterest.h',
  'Route.h',
  'RoutesController.h',
  'Uav.h',
  'UavController.h',
  'UavExecCommands.h',
  'UavErrors.h',
  'UavErrorsFilteredModel.h',
  'UavTrackable.h',
  'UavsTracker.h',
  'WaypointSettings.h',
  'UavUi.h',
  'UavUiController.h',
  'UserParam.h'
]

src = [
  'ChannelsController.cpp',
  'ExchangeService.cpp',
  'FirmwareWidgetPlugin.cpp',
  'GlobalActions.cpp',
  'Group.cpp',
  'GroupsController.cpp',
  'MotionExtension.cpp',
  'PlotData.cpp',
  'PointOfInterest.cpp',
  'Route.cpp',
  'RoutesController.cpp',
  'Structs.cpp',
  'Uav.cpp',
  'UavController.cpp',
  'UavExecCommands.cpp',
  'UavErrors.cpp',
  'UavErrorsFilteredModel.cpp',
  'UavIconGenerator.cpp',
  'UavTrackable.cpp',
  'UavsTracker.cpp',
  'WaypointSettings.cpp',
  'UavUi.cpp',
  'UavUiController.cpp',
  'UserParam.cpp'
]

processed = qt5_mod.preprocess(
  moc_headers : headers,
  include_directories : mcc_inc,
)

mcc_uav_lib = shared_library('mcc-uav',
  name_prefix : 'lib',
  sources : src + processed,
  include_directories : mcc_inc,
  link_with : [],
  dependencies : [qt5_core_dep, qt5_widgets_dep, qt5_gui_dep, bmcl_dep, fmt_dep, mcc_geo_dep, mcc_hm_dep, mcc_msg_dep, mcc_plugin_dep, mcc_path_dep, mcc_res_dep, mcc_ui_dep, libkml_dep],
  cpp_args : '-DBUILDING_MCC_UAV',
)

mcc_uav_dep = declare_dependency(
  link_with: mcc_uav_lib,
  include_directories: mcc_inc,
  dependencies: [mcc_geo_dep, mcc_ui_dep, qt5_gui_dep],
)

all_mcc_libs += mcc_uav_lib

