cfg = configuration_data()

plugin_list = '[\n'
foreach p : all_mcc_plugins
  plugin_list += '    r\''
  plugin_list += p.full_path()
  plugin_list += '\',\n'
endforeach
plugin_list += ']'

qml_plugins_list = '[\n'
foreach l : all_qml_plugins
  qml_plugins_list += '    r\''
  qml_plugins_list += l
  qml_plugins_list += '\',\n'
endforeach
qml_plugins_list += ']'

lib_list = '[\n'
foreach l : all_mcc_libs
  lib_list += '    r\''
  lib_list += l.full_path()
  lib_list += '\',\n'
endforeach
lib_list += ']'

tool_list = '[\n'
foreach t : all_mcc_tools
  tool_list += '    r\''
  tool_list += t.full_path()
  tool_list += '\',\n'
endforeach
tool_list += ']'

cfg.set('PLUGIN_LIST', plugin_list)
cfg.set('QML_PLUGINS_LIST', qml_plugins_list)
cfg.set('LIB_LIST', lib_list)
cfg.set('TOOL_LIST', tool_list)
cfg.set('BUILD_DIR', meson.build_root())
cfg.set('BIN_DIR', meson.current_build_dir())
cfg.set('SRC_DIR', meson.source_root())

cfg2 = cfg

copy_script = configure_file(
  configuration : cfg,
  input : 'copy.py.in',
  output : 'copy.py'
)

copy_outputs = []
copy_inputs = []

foreach l : all_mcc_libs + all_mcc_tools + all_mcc_plugins
    copy_inputs += l.full_path()
endforeach

foreach l : all_mcc_libs + all_mcc_tools
  parts = l.full_path().split('/')
  parts = parts[parts.length()-1]
  parts = parts.split('\\')
  base_name = parts[parts.length()-1]
  copy_outputs += base_name
endforeach

copy_target = custom_target('copy-mcc-deps',
  command : [python, copy_script],
  depends : all_mcc_libs + all_mcc_plugins + all_mcc_tools,
  output : copy_outputs,
)

mcc_link_deps = copy_target

mcc_exe = executable('mcc',
  sources : '../src/mcc/main.cpp',
  link_with : [mcc_ide_lib],
  link_depends : mcc_link_deps,
  include_directories : mcc_inc,
  dependencies : [bmcl_dep, qt5_core_dep, qt5_gui_dep, qt5_widgets_dep, tclap_dep, mcc_crashdump_dep, mcc_plugin_dep, mcc_path_dep, mcc_ui_dep],
  #gui_app : true,
)

cfg2.set('MCC_EXE', 'r\'' + mcc_exe.full_path() + '\'\n')

