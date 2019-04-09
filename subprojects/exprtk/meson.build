project('exprtk', ['cpp'])

exprtk_dep = declare_dependency(
  include_directories : include_directories('.', is_system : true)
)

if get_option('build_tests')
  test_exe = executable('exprtk_test',
    sources : 'exprtk_test.cpp',
    include_directories : include_directories('.')
  )

  test('exprtk_test', test_exe)
endif
