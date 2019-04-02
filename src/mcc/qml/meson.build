rc_file = 'qml.qrc'

moc_headers = [
    'DeviceUiWidget.h',
    'DeviceUiTool.h',
    'MjpegVideoSource.h',
    'MjpegVideoSourceTcp.h',
    'MjpegVideoSourceUdp.h',
    'QmlController.h',
    'QmlDataConverter.h',
    'QmlDeviceGroup.h',
    'QmlToolWindow.h',
    'QmlWrapper.h',
    'VideoImageProvider.h',
]

processed = qt5_mod.preprocess(
  moc_headers : moc_headers,
  include_directories : mcc_inc,
  qresources : rc_file,
)

src = [
    'DeviceUiWidget.cpp',
    'DeviceUiTool.cpp',
    'VideoImageProvider.cpp',
    'MjpegVideoSourceTcp.cpp',
    'MjpegVideoSourceUdp.cpp',
    'QmlController.cpp',
    'QmlDataConverter.cpp',
    'QmlToolWindow.cpp',
    'QmlWrapper.cpp',
]

mcc_qml_lib = shared_library('mcc-qml',
  name_prefix : 'lib',
  sources : src + processed,
  link_with : [mcc_ide_lib],
  include_directories : mcc_inc,
  dependencies : [bmcl_dep, qt5_core_dep, qt5_widgets_dep, qt5_quick_dep, qt5_network_dep, qt5_qml_dep, fmt_dep, qt5_gui_dep, mcc_msg_dep, mcc_plugin_dep, mcc_res_dep, mcc_uav_dep, mcc_ui_dep],
  cpp_args : '-DBUILDING_MCC_QML',
)

all_mcc_libs += mcc_qml_lib
