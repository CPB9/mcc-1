headers = [
  'Scene.h',
  'View.h',
]

src = [
  'BackgroundedTextItem.cpp',
  'BasicModels.cpp',
  'Methods.cpp',
  'ProductModel.cpp',
  'Scene.cpp',
  'TextInformer.cpp',
  'View.cpp',
]

processed = qt5_mod.preprocess(
  moc_headers : headers,
  include_directories : mcc_inc,
)

mcc_3d_lib = shared_library('mcc-3d',
  name_prefix : 'lib',
  sources : src + processed,
  link_with : [],
  include_directories : mcc_inc,
  dependencies : [vasnecov_dep, qt5_opengl_dep, qt5_core_dep, qt5_widgets_dep, qt5_gui_dep, bmcl_dep, fmt_dep, mcc_plugin_dep, mcc_path_dep, mcc_ui_dep, mcc_geo_dep],
  cpp_args : '-DBUILDING_MCC_3D',
)

all_mcc_libs += mcc_3d_lib
