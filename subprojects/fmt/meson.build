project('fmt', 'cpp', version : '5.2.1', default_options : ['cpp_std=c++11'])

check_code = '''
int main(int argc, char** argv)
{
    (void)argv;
#ifndef open
    return ((int*)(&${SYMBOL}))[argc];
#else
    (void)argc;
    return 0;
#endif
}
'''

cc = meson.get_compiler('cpp')

if target_machine.system() == 'windows'
  inc_prefix = '#include "io.h"\n'
else
  inc_prefix = '#include "fnctl.h"\n'
endif

have_open = cc.compiles(inc_prefix + check_code)

src = [
  'src/format.cc',
]

if have_open
  src += 'src/posix.cc'
endif

compile_args = []
interface_args = []
libtype = 'static_library'

if get_option('shared_lib')
  interface_args += '-DFMT_SHARED'
  compile_args += '-DFMT_EXPORT'
  libtype = 'shared_library'
endif

fmt_lib = build_target('fmt',
  include_directories : include_directories('include'),
  sources : src,
  target_type : libtype,
  name_prefix : 'lib',
  cpp_args : compile_args,
)

fmt_dep = declare_dependency(
  link_with : fmt_lib,
  include_directories : include_directories('include', is_system : true),
  version : '5.2.1',
  compile_args : interface_args,
)

