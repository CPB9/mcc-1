moc_headers = [
  'widgets/DeviceFirmwarePage.h',
  'widgets/FirmwareWidget.h',
  'widgets/MccNodeViewModel.h',
]

src = [
  'device/Device.cpp',
  'device/Device.h',
  'device/FileReader.cpp',
  'device/FileReader.h',
  'broker/Broker.h',
  'broker/Broker.cpp',
  'broker/Exchanger.h',
  'broker/Exchanger.cpp',
  'broker/Connections.h',
  'broker/Connections.cpp',
  'broker/Device.h',
  'broker/Device.cpp',
  'broker/Channel.cpp',
  'broker/Channel.h',
  'broker/Request.h',
  'traits/Trait.h',
  'traits/TraitRadioCalibration.cpp',
  'traits/TraitRadioCalibration.h',
  'widgets/DeviceFirmwarePage.cpp',
  'widgets/FirmwareWidget.cpp',
  'Firmware.h',
  'Firmware.cpp',
  'Plugin.cpp',
]

processed = qt5_mod.preprocess(
  moc_headers : moc_headers,
  include_directories : mcc_inc,
  qresources : 'net-photon.qrc',
)

plugin_mcc_net_photon = shared_library('plugin-mcc-net-photon',
  name_prefix : '',
  sources : src + processed,
  link_with : [mcc_plugin_net_lib, mcc_calib_lib],
  include_directories : mcc_inc,
  dependencies : [bmcl_dep, photon_dep, fmt_dep, mcc_msg_dep, libcaf_core_dep, mcc_error_dep, mcc_plugin_dep, mcc_uav_dep],
)

all_mcc_plugins += plugin_mcc_net_photon
