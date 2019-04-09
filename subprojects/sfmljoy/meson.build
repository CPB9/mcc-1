project('sfmljoy', 'cpp', default_options : 'cpp_std=c++11')

cc = meson.get_compiler('cpp')

src = [
  'src/sfmljoy/Clock.cpp',
  'src/sfmljoy/Err.cpp',
  'src/sfmljoy/Joystick.cpp',
  'src/sfmljoy/JoystickManager.cpp',
  'src/sfmljoy/Time.cpp',
]

deps = []
priv_args = []
pub_args = []

if host_machine.system() == 'windows'
  src += 'src/sfmljoy/Lock.cpp'
  src += 'src/sfmljoy/Mutex.cpp'
  src += 'src/sfmljoy/String.cpp'
  src += 'src/sfmljoy/Win32/ClockImpl.cpp'
  src += 'src/sfmljoy/Win32/MutexImpl.cpp'
  src += 'src/sfmljoy/Win32/JoystickImpl.cpp'
  deps += cc.find_library('winmm', required : true)
  priv_args += '-DUNICODE'
  priv_args += '-D_UNICODE'
elif host_machine.system() == 'linux'
  src += 'src/sfmljoy/Unix/ClockImpl.cpp'
  src += 'src/sfmljoy/Unix/JoystickImpl.cpp'
  deps += dependency('libudev', required : true)
#  deps += cc.find_library('rt', required : true)
  priv_args += '-fvisibility=hidden'
else
  error('Unsupported OS')
endif

deps += dependency('threads', required : true)
priv_args += '-DBUILDING_SFMLJOY'

sfmljoy_lib = shared_library('sfmljoy',
  name_prefix : 'lib',
  sources : src,
  include_directories : include_directories('include', 'src'),
  dependencies : deps,
  cpp_args : pub_args + priv_args,
)

sfmljoy_dep = declare_dependency(
  link_with : sfmljoy_lib,
  include_directories : include_directories('include', is_system : true),
  dependencies : deps,
  compile_args : pub_args,
)
