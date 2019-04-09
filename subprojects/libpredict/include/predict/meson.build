cfg = configuration_data()
cfg.set('PROJECT_VERSION_MAJOR', '2')
cfg.set('PROJECT_VERSION_MINOR', '0')
cfg.set('PROJECT_VERSION_PATCH', '0')
cfg.set('PROJECT_VERSION', '2.0.0')

predict_h = configure_file(
  configuration : cfg,
  input : 'predict.h.in',
  output : 'predict.h',
)
