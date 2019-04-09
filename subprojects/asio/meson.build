project('asio', 'cpp', default_options : ['cpp_std=c++11'])

thread_dep = dependency('threads')
deps = [thread_dep]

args = []
args += '-DASIO_STANDALONE'

if get_option('separate_compilation')
  args += '-DASIO_SEPARATE_COMPILATION'
  if get_option('shared_lib')
    args += '-DASIO_DYN_LINK'
    libtype = 'shared_library'
  else
    libtype = 'static_library'
  endif
endif

if target_machine.system() == 'windows'
  args += '-D_WIN32_WINNT=0x0601' #win7
  #args += '-DASIO_ENABLE_CANCELIO'
endif

if get_option('separate_compilation')
  asio_lib = build_target('asio',
    sources: 'asio/src/asio.cpp',
    name_prefix: 'lib',
    dependencies: deps,
    include_directories: include_directories('asio/include'),
    cpp_args: args,
    target_type : libtype,
  )
else
  asio_lib = []
endif

asio_dep = declare_dependency(
  link_with: asio_lib,
  include_directories: include_directories('asio/include', is_system : true),
  dependencies: deps,
  compile_args: args,
)
