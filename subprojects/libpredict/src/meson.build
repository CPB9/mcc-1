src = [
  'orbit.c',
  'sun.c',
  'observer.c',
  'sdp4.c',
  'sgp4.c',
  'refraction.c',
  'unsorted.c',
  'julian_date.c',
  'version.c',
  'moon.c',
  predict_h,
]

predict_cflags = []
deps = []

#TODO: windows build
if host_machine.system() == 'windows'
  predict_cflags += '-D_USE_MATH_DEFINES'
else
  predict_cflags += '-D_DEFAULT_SOURCE'
  cc = meson.get_compiler('c')
  deps += cc.find_library('m')
endif

predict_lib = static_library('predict',
  name_prefix : 'lib',
  sources : src,
  include_directories : include_directories('../include'),
  c_args : predict_cflags,
  dependencies : deps,
)

predict_dep = declare_dependency(
  link_with : predict_lib,
  include_directories : include_directories('../include', is_system : true),
)
