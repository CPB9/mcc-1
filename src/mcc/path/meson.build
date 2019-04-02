src = [
  'Paths.cpp',
]

deps = [bmcl_dep]

mcc_path_lib = shared_library('mcc-path',
  name_prefix : 'lib',
  sources : src,
  dependencies : deps,
  include_directories : mcc_inc,
  cpp_args : '-DBUILDING_MCC_PATH',
)

mcc_path_dep = declare_dependency(
  link_with : mcc_path_lib,
  include_directories : mcc_inc,
  dependencies : deps,
)

all_mcc_libs += mcc_path_lib
