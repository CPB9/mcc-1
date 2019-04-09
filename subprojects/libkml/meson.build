project('libkml', ['c', 'cpp'], default_options : ['cpp_std=c++11', 'c_std=c99'])

bmcl_mod = subproject('bmcl', default_options: ['build_tests=false'])
bmcl_dep = bmcl_mod.get_variable('bmcl_dep')

subdir('third_party/zlib-1.2.8')
subdir('third_party/expat-2.2.5')
subdir('third_party/uriparser-0.7.5')
subdir('third_party/kml-minizip/minizip')

subdir('src/kml')
