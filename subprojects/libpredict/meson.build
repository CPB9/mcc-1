project('predict', ['c', 'cpp'], version : '2.0.0', default_options : ['c_std=c99', 'cpp_std=c++98'])

subdir('include/predict')
subdir('src')

if get_option('build_tests')
  subdir('tests')
endif
