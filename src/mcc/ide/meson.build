cfg = configuration_data()
cfg.set('IDE_RC_DIR', join_paths(meson.current_source_dir(), 'resources'))

rc_file = configure_file(
  configuration : cfg,
  input : 'ide.qrc.in.meson',
  output : 'ide.qrc',
)

ui_files = [
  'uifiles/SerialConnectionWidget.ui',
  'uifiles/UdpConnectionWidget.ui',
#  uifiles/MccPlotTool.ui
]

moc_headers = [
  'dialogs/AddChannelDialog.h',
  'dialogs/AddUavDialog.h',
  'models/DevicesListModel.h',
  'models/LogMessagesModel.h',
  'models/SettingsTreeModel.h',
  'toolbar/AddEntityWidget.h',
  'toolbar/MainToolBar.h',
  'view/AbstractConnectionWidget.h',
  'view/ContainerWidget.h',
  'view/MainWindow.h',
  'view/MccPlot.h',
  'view/NetStatisticsWidget.h',
  'view/PlotWidget.h',
  'view/SerialConnectionWidget.h',
  'view/UavBrowserTool.h',
]

processed = qt5_mod.preprocess(
  qresources : rc_file,
  ui_files : ui_files,
  moc_headers : moc_headers,
  include_directories : mcc_inc,
)

src = [
  'dialogs/AddChannelDialog.cpp',
  'dialogs/AddUavDialog.cpp',
  'dialogs/SettingsDialog.cpp',
  'models/DevicesListModel.cpp',
  'models/LogMessagesModel.cpp',
  'models/SettingsTreeModel.cpp',
  'toolbar/AddEntityWidget.cpp',
  'toolbar/MainToolBar.cpp',
  'view/ContainerWidget.cpp',
  'view/MainWindow.cpp',
  'view/MccPlot.cpp',
  'view/NetStatisticsWidget.cpp',
  'view/PlotWidget.cpp',
  'view/SerialConnectionWidget.cpp',
  'view/UavBrowserTool.cpp',
  'view/UdpConnectionWidget.cpp',
  'view/ZipExtractor.cpp',
]

#set(IDE_VIEW_ICON_RC)
#if (WIN32)
#     set(IDE_VIEW_ICON_RC ${IDE_VIEW_DIR}/resources/ide.rc)
#endif ()

mcc_ide_lib = shared_library('mcc-ide',
  name_prefix : 'lib',
  sources : src + processed,
  link_with : [],
  include_directories : mcc_inc,
  dependencies : [bmcl_dep, fmt_dep, qt5_core_dep, qt5_gui_dep, qt5_widgets_dep, qt5_serialport_dep, qwt_dep, ads_dep, libkml_dep, mcc_msg_dep, mcc_plugin_dep, mcc_path_dep, mcc_res_dep, mcc_uav_dep],
  cpp_args : '-DBUILDING_MCC_IDE',
)

all_mcc_libs += mcc_ide_lib
