project('zpaq', 'cpp')

args = []
if get_option('buildtype').startswith('debug')
  args += '-DDEBUG'
endif

if target_machine.system() != 'windows'
  args += '-Dunix'
endif

if (not get_option('enable_jit')) or (not target_machine.cpu_family().startswith('x86'))
  args += '-DNOJIT'
endif

inc = include_directories('.')

zpaq_lib = static_library('zpaq',
  sources: 'libzpaq.cpp',
  name_prefix: 'lib',
  cpp_args: args,
  include_directories: inc,
)

libzpaq_dep = declare_dependency(
  link_with: zpaq_lib,
  include_directories: inc,
)

thread_dep = dependency('threads')

zpaq_exe = executable('zpaq',
  sources: 'zpaq.cpp',
  dependencies: [libzpaq_dep, thread_dep],
)
