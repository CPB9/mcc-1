deps = [bmcl_dep, qt5_gui_dep, qt5_svg_dep]

res_gen = executable('mcc-res-gen',
  sources : 'resgen.cpp',
  dependencies : deps + [rapidjson_dep],
)

res_generated_src = custom_target('mcc-res-generated-src',
  input : 'default_resources.json',
  output : ['_generated_ResourceImpl.cpp', '_generated_ResourceKind.h'],
  depend_files : mcc_module_files,
  depends : replaced_resources,
  command : [res_gen,
             '@INPUT@',
             replaced_resources.full_path(),
             meson.current_source_dir(),
             meson.source_root(),
             meson.current_build_dir()],
)

src = [
  'Resource.cpp',
  'Resource.h',
  res_generated_src,
]

mcc_res_lib = shared_library('mcc-res',
  name_prefix : 'lib',
  sources : src,
  include_directories : mcc_inc,
  dependencies : deps,
  cpp_args : '-DBUILDING_MCC_RES',
)

mcc_res_dep = declare_dependency(
  link_with : mcc_res_lib,
  include_directories : mcc_inc,
  dependencies : deps,
  sources : res_generated_src[1],
)

all_mcc_libs += mcc_res_lib
