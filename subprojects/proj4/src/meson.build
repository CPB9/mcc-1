cfg = configuration_data()

deps = [dependency('threads')]

cfg.set('HAVE_INTTYPES_H', 1)
cfg.set('HAVE_LOCALECONV', 1)
cfg.set('HAVE_MEMORY_H', 1)
cfg.set('HAVE_STDINT_H', 1)
cfg.set('HAVE_STDLIB_H', 1)
cfg.set('HAVE_STRING_H', 1)
cfg.set('HAVE_SYS_STAT_H', 1)
cfg.set('HAVE_SYS_TYPES_H', 1)
cfg.set_quoted('PACKAGE', 'proj')
cfg.set_quoted('PACKAGE_BUGREPORT', 'https://github.com/OSGeo/proj.4/issues')
cfg.set_quoted('PACKAGE_NAME', 'PROJ')
cfg.set_quoted('PACKAGE_STRING', 'PROJ 5.2.0')
cfg.set_quoted('PACKAGE_TARNAME', 'proj')
cfg.set_quoted('PACKAGE_VERSION', '5.2.0')
cfg.set('STDC_HEADERS', 1)

if host_machine.system() != 'windows'
  cc = meson.get_compiler('c')
  cfg.set('HAVE_UNISTD_H', 1)
  cfg.set('HAVE_DLFCN_H', 1)
  cfg.set('HAVE_LIBM', 1)
  deps += cc.find_library('m', required : true)
endif

config_h = configure_file(
  input : 'proj_config.meson.in',
  output : 'proj_config.h',
  configuration : cfg,
)

src = [
  config_h,
  'nad_init.c',
  'PJ_aea.c',
  'PJ_aeqd.c',
  'PJ_airy.c',
  'PJ_aitoff.c',
  'PJ_august.c',
  'PJ_axisswap.c',
  'PJ_bacon.c',
  'PJ_bipc.c',
  'PJ_boggs.c',
  'PJ_bonne.c',
  'PJ_calcofi.c',
  'PJ_cart.c',
  'PJ_cass.c',
  'PJ_cc.c',
  'PJ_ccon.c',
  'PJ_cea.c',
  'PJ_chamb.c',
  'PJ_collg.c',
  'PJ_comill.c',
  'PJ_crast.c',
  'PJ_deformation.c',
  'PJ_denoy.c',
  'PJ_eck1.c',
  'PJ_eck2.c',
  'PJ_eck3.c',
  'PJ_eck4.c',
  'PJ_eck5.c',
  'PJ_eqc.c',
  'PJ_eqdc.c',
  'PJ_eqearth.c',
  'PJ_fahey.c',
  'PJ_fouc_s.c',
  'PJ_gall.c',
  'PJ_geoc.c',
  'PJ_geos.c',
  'PJ_gins8.c',
  'PJ_gnom.c',
  'PJ_gn_sinu.c',
  'PJ_goode.c',
  'PJ_gstmerc.c',
  'PJ_hammer.c',
  'PJ_hatano.c',
  'PJ_helmert.c',
  'PJ_hgridshift.c',
  'PJ_horner.c',
  'PJ_igh.c',
  'PJ_isea.c',
  'PJ_imw_p.c',
  'PJ_krovak.c',
  'PJ_labrd.c',
  'PJ_laea.c',
  'PJ_lagrng.c',
  'PJ_larr.c',
  'PJ_lask.c',
  'PJ_latlong.c',
  'PJ_lcca.c',
  'PJ_lcc.c',
  'PJ_loxim.c',
  'PJ_lsat.c',
  'PJ_misrsom.c',
  'PJ_mbt_fps.c',
  'PJ_mbtfpp.c',
  'PJ_mbtfpq.c',
  'PJ_merc.c',
  'PJ_mill.c',
  'PJ_mod_ster.c',
  'PJ_moll.c',
  'PJ_molodensky.c',
  'PJ_natearth.c',
  'PJ_natearth2.c',
  'PJ_nell.c',
  'PJ_nell_h.c',
  'PJ_nocol.c',
  'PJ_nsper.c',
  'PJ_nzmg.c',
  'PJ_ob_tran.c',
  'PJ_ocea.c',
  'PJ_oea.c',
  'PJ_omerc.c',
  'PJ_ortho.c',
  'PJ_patterson.c',
  'PJ_pipeline.c',
  'PJ_poly.c',
  'PJ_putp2.c',
  'PJ_putp3.c',
  'PJ_putp4p.c',
  'PJ_putp5.c',
  'PJ_putp6.c',
  'PJ_qsc.c',
  'PJ_robin.c',
  'PJ_rpoly.c',
  'PJ_sch.c',
  'PJ_sconics.c',
  'PJ_somerc.c',
  'PJ_sterea.c',
  'PJ_stere.c',
  'PJ_sts.c',
  'PJ_tcc.c',
  'PJ_tcea.c',
  'PJ_times.c',
  'PJ_tmerc.c',
  'PJ_tpeqd.c',
  'PJ_unitconvert.c',
  'PJ_urm5.c',
  'PJ_urmfps.c',
  'PJ_vandg.c',
  'PJ_vandg2.c',
  'PJ_vandg4.c',
  'PJ_vgridshift.c',
  'PJ_wag2.c',
  'PJ_wag3.c',
  'PJ_wag7.c',
  'PJ_wink1.c',
  'PJ_wink2.c',
  'proj_etmerc.c',
  'aasincos.c',
  'adjlon.c',
  'bch2bps.c',
  'bchgen.c',
  'biveval.c',
  'dmstor.c',
  'emess.c',
  'geocent.c',
  'geodesic.c',
  'mk_cheby.c',
  'nad_cvt.c',
  'nad_init.c',
  'nad_intr.c',
  'pj_apply_gridshift.c',
  'pj_apply_vgridshift.c',
  'pj_auth.c',
  'pj_ctx.c',
  'pj_fileapi.c',
  'pj_datum_set.c',
  'pj_datums.c',
  'pj_deriv.c',
  'pj_ell_set.c',
  'pj_ellps.c',
  'pj_errno.c',
  'pj_factors.c',
  'pj_fwd.c',
  'pj_gauss.c',
  'pj_gc_reader.c',
  'pj_geocent.c',
  'pj_gridcatalog.c',
  'pj_gridinfo.c',
  'pj_gridlist.c',
  'PJ_healpix.c',
  'pj_init.c',
  'pj_initcache.c',
  'pj_inv.c',
  'pj_list.c',
  'pj_log.c',
  'pj_malloc.c',
  'pj_math.c',
  'pj_mlfn.c',
  'pj_msfn.c',
  'pj_mutex.c',
  'proj_4D_api.c',
  'pj_internal.c',
  'pj_open_lib.c',
  'pj_param.c',
  'pj_phi2.c',
  'pj_pr_list.c',
  'pj_qsfn.c',
  'pj_release.c',
  'pj_strerrno.c',
  'pj_transform.c',
  'pj_tsfn.c',
  'pj_units.c',
  'pj_utils.c',
  'pj_zpoly1.c',
  'proj_mdist.c',
  'proj_rouss.c',
  'rtodms.c',
  'vector1.c',
  'pj_strtod.c',
]

c_args = ['-DHAVE_C99_MATH=1', '-Dproj_EXPORTS']
link_args = []
if build_machine.system() == 'windows'
  c_args += '-DMUTEX_win32'
  link_args += '/DEF:' + join_paths(meson.current_source_dir(), 'proj.def')
else
  c_args += '-DMUTEX_pthread'
endif

proj_lib = shared_library('proj',
  name_prefix : 'lib',
  sources : src,
  c_args : c_args,
  link_args : link_args,
  include_directories : include_directories('.'),
  dependencies : deps,
)

proj_dep = declare_dependency(
  link_with : proj_lib,
  include_directories : include_directories('.', is_system : true),
)
