defs = ['-DADS_SHARED_EXPORT', '-DQT_NO_DEBUG_OUTPUT']

moc_headers = [
    'ads_globals.h',
    'DockAreaWidget.h',
    'DockAreaTabBar.h',
    'DockManager.h',
    'DockWidget.h',
    'DockWidgetTab.h',
    'FloatingDockContainer.h',
    'DockOverlay.h',
    'DockSplitter.h',
    'DockAreaTitleBar.h',
    'ElidingLabel.h',
]

src = [
    'ads_globals.cpp',
    'DockAreaWidget.cpp',
    'DockAreaTabBar.cpp',
    'DockContainerWidget.cpp',
    'DockManager.cpp',
    'DockWidget.cpp',
    'DockWidgetTab.cpp',
    'FloatingDockContainer.cpp',
    'DockOverlay.cpp',
    'DockSplitter.cpp',
    'DockAreaTitleBar.cpp',
    'ElidingLabel.cpp',
]

rc_files = 'ads.qrc'

processed = qt5_mod.preprocess(
  qresources : rc_files,
  moc_headers : moc_headers,
)

moc = find_program('moc')

processed2 = custom_target('custom_moc',
  input : ['DockContainerWidget.h'],
  output : ['moc_DockContainerWidget.cpp.h'],
  command : [moc, '@INPUT@', '-o', '@OUTPUT@'],
)

ads_lib = shared_library('ads',
  sources : src + processed + [processed2],
  cpp_args : defs,
  include_directories : include_directories('.'),
  dependencies : qt5_dep,
)

ads_dep = declare_dependency(
  link_with : ads_lib,
  include_directories : include_directories('.', is_system : true),
)
