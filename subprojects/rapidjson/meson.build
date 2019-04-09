project('rapidjson', 'cpp', version : '1.1.0', default_options : ['cpp_std=c++11'])

inc = include_directories('include')

rapidjson_dep = declare_dependency(
  include_directories : inc,
  compile_args : '-DRAPIDJSON_HAS_STDSTRING',
  version : '1.1.0',
)
