deps = []
if host_machine.system() == 'windows'
  dbg_lib = meson.get_compiler('c').find_library('Dbghelp')
  if dbg_lib.found()
    src = ['CrashDump_Win32.cpp']
    deps += dbg_lib
  else
    src = ['CrashDump_Dummy.cpp']
  endif
else
  src = ['CrashDump_Dummy.cpp']
endif

headers = ['CrashDump.h']

mcc_crashdump_lib = shared_library('mcc-crashdump',
  name_prefix : 'lib',
  sources : src + headers,
  cpp_args : '-DBUILDING_MCC_CRASHDUMP',
  include_directories : mcc_inc,
  dependencies : deps,
)

mcc_crashdump_dep = declare_dependency(
  link_with : mcc_crashdump_lib,
  include_directories : mcc_inc,
)

all_mcc_libs += mcc_crashdump_lib
