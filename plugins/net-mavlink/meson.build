moc_headers = [
  'widgets/MavlinkParametersWidget.h',
  'widgets/MavlinkMonitorWidget.h',
  'widgets/FirmwareWidget.h',
  'widgets/FirmwareModel.h',
  'widgets/ParameterEditor.h',
  'widgets/MavlinkToolbarWidget.h',
  'widgets/AirframesModel.h',
  'widgets/AirframeWidget.h',
  'widgets/AirframeViewDelegate.h'
]

processed = qt5_mod.preprocess(
  moc_headers : moc_headers,
  include_directories : mcc_inc,
  qresources : 'net-mavlink.qrc',
)

src = [
  'broker/Broker.h',
  'broker/Broker.cpp',
  'broker/Channel.h',
  'broker/Channel.cpp',
  'broker/Connections.h',
  'broker/Connections.cpp',
  'broker/Device.h',
  'broker/Device.cpp',
  'broker/Exchanger.h',
  'broker/LogWriter.h',
  'broker/LogWriter.cpp',
  'broker/Request.h',
  'device/Device.cpp',
  'device/Device.h',
  'device/Px4Fsm.h',
  'device/Px4Fsm.cpp',
  'device/Tm.h',
  'device/Tm.cpp',
  'device/Mavlink.cpp',
  'device/Mavlink.h',
  'device/MavlinkUtils.h',
  'device/MavlinkUtils.cpp',
  'device/px4_custom_mode.h',
  'device/Route.h',
  'traits/Trait.h',
  'traits/TraitJoystick.h',
  'traits/TraitJoystick.cpp',
  'traits/TraitParams.h',
  'traits/TraitParams.cpp',
  'traits/TraitRadioCalibration.h',
  'traits/TraitRadioCalibration.cpp',
  'traits/TraitRegistrator.h',
  'traits/TraitRegistrator.cpp',
  'traits/TraitRoutes.h',
  'traits/TraitRoutes.cpp',
  'traits/TraitSensorCalibration.h',
  'traits/TraitSensorCalibration.cpp',
  'widgets/FirmwareModel.cpp',
  'widgets/FirmwareWidget.cpp',
  'widgets/MavlinkMonitor.h',
  'widgets/MavlinkMonitorWidget.cpp',
  'widgets/MavlinkParametersWidget.cpp',
  'widgets/ParameterEditor.cpp',
  'widgets/MavlinkToolbarWidget.cpp',
  'widgets/AirframesModel.cpp',
  'widgets/AirframeWidget.cpp',
  'Firmware.h',
  'Firmware.cpp',
  'Plugin.cpp',
]

extra = [
  'device/PX4ParameterFactMetaData.xml',
]

plugin_mcc_net_mavlink = shared_library('plugin-mcc-net-mavlink',
  name_prefix : '',
  sources : src + processed,
  extra_files: extra,
  link_with : [mcc_plugin_net_lib, mcc_calib_lib],
  include_directories : mcc_inc,
  dependencies : [qt5_xml_dep, rapidjson_dep, bmcl_dep, qt5_core_dep, qt5_widgets_dep, fmt_dep, mavlink2_dep, qt5_gui_dep, mcc_msg_dep, libcaf_core_dep, mcc_error_dep, mcc_plugin_dep, mcc_path_dep, mcc_res_dep, mcc_uav_dep],
)

all_mcc_plugins += plugin_mcc_net_mavlink
