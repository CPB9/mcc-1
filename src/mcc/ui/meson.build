headers = [
  'ClickableLabel.h',
  'ColorButton.h',
  'CoordinateEditor.h',
  'CoordinateSystemController.h',
  'Dialog.h',
  'FastEditDoubleSpinBox.h',
  'HeightmapController.h',
  'LatLonEditor.h',
  'QProgressIndicator.h',
  'Settings.h',
  'SettingsPage.h',
  'SliderCheckBox.h',
  'TableEditWidget.h',
  'TextualProgressIndicator.h',
  'Trackable.h',
  'VariantButton.h',
  'WidgetUtils.h',
  'FileUtils.h',
  'UserNotifier.h'
]

src = [
  'ClickableLabel.cpp',
  'ColorButton.cpp',
  'ColorDialogOptions.cpp',
  'CoordinateEditor.cpp',
  'CoordinateFormat.cpp',
  'CoordinateFormatter.cpp',
  'CoordinateSystem.cpp',
  'CoordinateSystemController.cpp',
  'Dialog.cpp',
  'DialogPlugin.cpp',
  'FastEditDoubleSpinBox.cpp',
  'FlowLayout.cpp',
  'GraphicsEffectCreator.cpp',
  'Gradient.cpp',
  'HeightmapController.cpp',
  'LatLonEditor.cpp',
  'MainMenuPlugin.cpp',
  'QProgressIndicator.cpp',
  'Settings.cpp',
  'SettingsPage.cpp',
  'SettingsPagePlugin.cpp',
  'SliderCheckBox.cpp',
  'TableEditWidget.cpp',
  'TextUtils.cpp',
  'TextualProgressIndicator.cpp',
  'Trackable.cpp',
  'TreeViewWithKeyboard.cpp',
  'UiPlugin.cpp',
  'VariantButton.cpp',
  'WidgetPlugin.cpp',
  'WidgetUtils.cpp',
  'FileUtils.cpp',
  'UserNotifier.cpp'
]

processed = qt5_mod.preprocess(
  moc_headers : headers,
  include_directories : mcc_inc,
)

ext_deps = []
if qt5_text_to_speech_dep.found()
  ext_deps += [qt5_text_to_speech_dep]
endif

mcc_ui_lib = shared_library('mcc-ui',
  name_prefix : 'lib',
  sources : src + processed,
  include_directories : mcc_inc,
  dependencies : [qt5_core_dep, qt5_widgets_dep, qt5_gui_dep, qt5_multimedia_dep, bmcl_dep, fmt_dep, mcc_geo_dep, mcc_plugin_dep, mcc_path_dep, mcc_res_dep] + ext_deps,
  cpp_args : '-DBUILDING_MCC_UI',
)

mcc_ui_dep = declare_dependency(
  link_with: mcc_ui_lib,
  include_directories: mcc_inc,
  dependencies: [mcc_geo_dep, qt5_widgets_dep],
)

all_mcc_libs += mcc_ui_lib
