project('lz4', 'c', version : '1.8.3')

src = [
  'lib/lz4.c',
  'lib/lz4frame.c',
  'lib/lz4hc.c',
  'lib/xxhash.c',
]

lz4_lib = static_library('lz4',
  name_prefix : 'lib',
  sources : src,
  include_directories : include_directories('lib'),
  c_args: '-DXXH_NAMESPACE=LZ4_',
)

lz4_dep = declare_dependency(
  link_with : lz4_lib,
  include_directories : include_directories('lib', is_system : true),
  version : '1.8.3',
)
