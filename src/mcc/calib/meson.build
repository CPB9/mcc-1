moc_headers = [
    'SensorCalibrationWidget.h',
    'IndicatorWidget.h',
    'CalibrationDialog.h',
    'CalibrationDialogPage.h',
    'CalibrationControllerAbstract.h',
    'RadioCalibrationWidget.h',
    'EscCalibrationWidget.h',
    'SimpleFlightModesWidget.h',
]

rc = [
  'px4.qrc',
]

processed = qt5_mod.preprocess(
  moc_headers : moc_headers,
  include_directories : mcc_inc,
  qresources : rc,
)

src = [
    'SensorCalibrationWidget.cpp',
    'IndicatorWidget.cpp',
    'CalibrationDialog.cpp',
    'RadioCalibrationWidget.cpp',
    'EscCalibrationWidget.cpp',
    'SimpleFlightModesWidget.cpp',
]

mcc_calib_lib = shared_library('mcc-calib',
  name_prefix : 'lib',
  sources : src + processed,
  link_with : [],
  include_directories : mcc_inc,
  dependencies : [qt5_xml_dep, rapidjson_dep, bmcl_dep, qt5_core_dep, qt5_widgets_dep, fmt_dep, mavlink2_dep, qt5_gui_dep, mcc_msg_dep, mcc_plugin_dep, mcc_res_dep, mcc_uav_dep],
  cpp_args : '-DBUILDING_MCC_CALIB',
)

all_mcc_libs += mcc_calib_lib
