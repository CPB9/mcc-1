project('caf', 'cpp', default_options : ['cpp_std=c++11'], version : '0.16.0')

cc = meson.get_compiler('cpp')

cc_id = cc.get_id()

cflags = []

if cc_id == 'clang'
  cflags += ['-ftemplate-depth=512', '-ftemplate-backtrace-limit=0', '-Wno-deprecated-declarations']
elif cc_id == 'gcc'
  cflags += ['-Wno-missing-field-initializers', '-Wno-deprecated-declarations']
endif

shared_deps = [dependency('threads')]

system = host_machine.system()

if system == 'windows' and cc_id == 'gcc'
  cflags += ['-D_WIN32_WINNT=0x0600', '-DWIN32']
elif system == 'cygwin'
  cflags += '-U__STRICT_ANSI__'
endif

if system == 'windows'
  shared_deps += [cc.find_library('ws2_32'),
                  cc.find_library('iphlpapi'),
                  cc.find_library('psapi')]
endif

subdir('libcaf_core')
