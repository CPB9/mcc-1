src = [
  'Sql.h',
  'DbObj.h',
  'DbObj.cpp',
  'DbHandler.cpp',
  'DbHandler.h',
  'obj/DbObjInternal.h',
  'obj/Object.h',
  'obj/Object.cpp',
  'obj/Advanced.h',
  'obj/Advanced.cpp',
  'obj/Channel.h',
  'obj/Channel.cpp',
  'obj/Device.h',
  'obj/Device.cpp',
  'obj/DeviceUi.h',
  'obj/DeviceUi.cpp',
  'obj/Firmware.h',
  'obj/Firmware.cpp',
  'obj/Protocol.h',
  'obj/Protocol.cpp',
  'obj/Radar.h',
  'obj/Radar.cpp',
  'obj/TmSession.h',
  'obj/TmSession.cpp',
  'obj/DbObjInternal.cpp',
]

src_extra = [
  'obj/Schema.sql',
]

db_rc = qt5_mod.preprocess(
  qresources  : 'Db.qrc',
)

mcc_db_lib = shared_library('mcc-db',
  name_prefix : 'lib',
  sources : src + db_rc,
  extra_files: src_extra,
  link_with : [mcc_plugin_net_lib],
  include_directories : mcc_inc,
  dependencies : [bmcl_dep, qt5_core_dep, fmt_dep, sqlite3pp_dep, mcc_msg_dep, mcc_error_dep, mcc_plugin_dep, mcc_path_dep, libcaf_core_dep],
  cpp_args : '-DBUILDING_MCC_DB',
)

mcc_db_dep = declare_dependency(link_with: mcc_db_lib,
  include_directories: mcc_inc,
  dependencies: mcc_msg_dep
)

all_mcc_libs += mcc_db_lib
