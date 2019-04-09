src = [
  'unzip.c',
  'ioapi.c',
  'iomem_simple.c',
  'zip.c',
]

inc = include_directories('.')

minizip_lib = static_library('minizip',
  sources : src,
  dependencies : zlib_dep,
  include_directories : inc,
)

dep_inc = include_directories('..')

minizip_dep = declare_dependency(
  link_with : minizip_lib,
  include_directories : dep_inc,
  dependencies : zlib_dep,
)
