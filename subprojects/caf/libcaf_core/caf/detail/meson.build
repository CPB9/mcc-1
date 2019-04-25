conf_data = configuration_data()

conf_data.set('CAF_LOG_LEVEL', get_option('log_level').to_int())

build_config = configure_file(
  input : 'build_config.hpp.in',
  output : 'build_config.hpp',
  configuration : conf_data
)

