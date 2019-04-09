project('mcc', ['c', 'cpp'], default_options : ['cpp_std=c++14', 'c_std=c99', 'backend_startup_project=mcc'])

cc = meson.get_compiler('cpp')

all_mcc_plugins = []
all_mcc_libs = []
all_mcc_tools = []
all_qml_plugins = []

global_args_c = []
global_args_cpp = []
proj_args_c = []
proj_args_cpp = []
global_link_args = []

if cc.get_id() == 'clang' or cc.get_id() == 'gcc'
  proj_args = ['-fvisibility=hidden']
  global_args_c = ['-Werror=return-type']
  global_args_cpp = ['-Wno-reorder', '-Werror=return-type']
  global_link_args = ['-Wl,-rpath,./bin', '-Wl,-rpath,.']
else
  global_args_c = ['-wd4251', '-wd4275', '-wd4910']
  if get_option('buildtype') == 'release'
    global_args_c += '-GL'
    global_link_args += '-LTCG'
  endif
  global_args_cpp = global_args_c
endif

if cc.get_id() == 'clang' or cc.get_id() == 'gcc'
  foreach arg : ['-Werror=invalid-source-encoding',
                 '-Werror=return-local-addr',
                 '-Werror=return-stack-address',
                 '-Werror=return-std-move',
                 '-Werror=delete-incomplete',
                 '-Werror=pessimizing-move',
                 '-Werror=inconsistent-missing-override',
                 '-Werror=switch',
                 '-Werror=pragma-once-outside-header',
                 '-Werror=overloaded-virtual',
                 '-Werror=unused-variable',
                 '-Werror=unused-value',
                 '-Werror=unused-private-field'
                 ]
    if cc.has_argument(arg)
      proj_args_c += arg
      proj_args_cpp += arg
    endif
  endforeach
  foreach arg : ['-Wno-unused-lambda-capture'
                ]
    if cc.has_argument(arg)
      global_args_c += arg
      global_args_cpp += arg
    endif
  endforeach
endif

add_project_arguments(proj_args_c, language : ['c'])
add_project_arguments(proj_args_cpp, language : ['cpp'])
add_global_arguments(global_args_c, language : ['c'])
add_global_arguments(global_args_cpp, language : ['cpp'])
add_global_link_arguments(global_link_args, language : ['c', 'cpp'])

thread_dep = dependency('threads')

qt5_mod = import('qt5')
qt5_core_dep = dependency('qt5', modules: ['Core'])
qt5_gui_dep = dependency('qt5', modules: ['Gui'])
qt5_widgets_dep = dependency('qt5', modules: ['Widgets'])
qt5_svg_dep = dependency('qt5', modules: ['Svg'])
qt5_network_dep = dependency('qt5', modules: ['Network'])
qt5_positioning_dep = dependency('qt5', modules: ['Positioning'])
qt5_serialport_dep = dependency('qt5', modules: ['SerialPort'])
qt5_xml_dep = dependency('qt5', modules: ['Xml'])
qt5_opengl_dep = dependency('qt5', modules: ['OpenGL'])
qt5_quick_dep = dependency('qt5', modules: ['Quick'])
qt5_qml_dep = dependency('qt5', modules: ['Qml'])
qt5_printsupport_dep = dependency('qt5', modules: ['PrintSupport'])
qt5_text_to_speech_dep = dependency('qt5', modules: ['TextToSpeech'], disabler: true, required: false)
qt5_multimedia_dep = dependency('qt5', modules: ['Multimedia'])
omp_dep = dependency('openmp', required : true)
gdal_dep = dependency('gdal', version : '>=2.2', required : false)

#  'Multimedia',
#  'QuickWidgets',
#  'PrintSupport',

cfg = configuration_data()

cfg.set('MESON_0_47_0_DUMMY_FIX', 1) #HACK

build_num = get_option('build_number')
if build_num != 0
  git = find_program('git', required : true)
  r = run_command(git, 'rev-parse', 'HEAD')
  if r.returncode() != 0
    error('Failed to run git: ' + r.stderr())
  endif
  cfg.set_quoted('MCC_COMMIT_HASH', r.stdout().strip())
  r = run_command(git, 'rev-parse', '--abbrev-ref', 'HEAD')
  if r.returncode() != 0
    error('Failed to run git: ' + r.stderr())
  endif
  cfg.set_quoted('MCC_BRANCH', r.stdout().strip())
  cfg.set('MCC_BUILD_NUMBER', build_num)
endif

# for deploy config only
subdir('src/mcc')

mcc_inc = include_directories('src')

python = find_program('python3', required : false)

if not python.found()
  python = find_program('python', required : true)
endif

mcc_module_files = files(get_option('modules'))

mod_str = run_command(python, 'list-modules.py', 'modules', get_option('modules'))

replaced_resources = custom_target('mcc-gen-resource-replacements',
  input : get_option('modules'),
  output : 'replaced_resources.json',
  capture : true,
  depend_files : mcc_module_files,
  command : [python,
             join_paths(meson.current_source_dir(), 'list-modules.py'),
             'resources',
             '@INPUT@'],
)

if mod_str.returncode() != 0
  error('failed to list modules')
endif

foreach line : mod_str.stdout().strip().split('\n')
  parts = line.split('|')
  dep_name = parts[0].strip()
  sub_name = parts[1].strip()
  var = parts[2].strip()
  lib_str = parts[3].strip()
  tool_str = parts[4].strip()
  opt_str = parts[5].strip()

  dep_set = false

  if dep_name != ''
    dep = dependency(dep_name, required : sub_name == '')
    if dep.found()
      set_variable(var, dep)
      dep_set = true
    endif
  endif

  if not dep_set
    opts = []
    foreach o : opt_str.strip().split(',')
      if o != ''
        opts += o.strip()
      endif
    endforeach

    mod_name = sub_name + '_mod'
    set_variable(mod_name, subproject(sub_name, default_options: opts))
    if var != ''
      set_variable(var, get_variable(mod_name).get_variable(var))
    endif

    foreach l : lib_str.strip().split(',')
      if l != ''
        all_mcc_libs += get_variable(mod_name).get_variable(l.strip())
      endif
    endforeach

    foreach t : tool_str.strip().split(',')
      if t != ''
        all_mcc_tools += get_variable(mod_name).get_variable(t.strip())
      endif
    endforeach
  endif
endforeach

subdir_str = run_command(python, 'list-modules.py', 'subdirs', get_option('modules'))

if subdir_str.returncode() != 0
  error('failed to list subdirs')
endif


foreach line : subdir_str.stdout().split('\n')
  line = line.strip()
  if line != '' and line[0] != '#'
    subdir(line)
  endif
endforeach

subdir('bin')
subdir('deploy')
subdir('tests')
