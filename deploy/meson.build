
deploy_script = configure_file(
  configuration : cfg2,
  input : 'deploy.py.in',
  output : 'deploy.py'
)

run_target('deploy-mcc',
  command : [python, deploy_script],
  depends : mcc_exe,
)
