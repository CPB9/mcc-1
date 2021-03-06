kml_inc = include_directories('..')

file_src = []
if target_machine.system() == 'windows'
    file_src += 'base/file_win32.cc'
    file_src += 'base/missing/strptime.cc'
else
    file_src += 'base/file_posix.cc'
endif

src = [
  'base/attributes.cc',
  'base/csv_splitter.cc',
  'base/date_time.cc',
  'base/expat_handler.cc',
  'base/expat_handler_ns.cc',
  'base/expat_parser.cc',
  'base/file.cc',
  'base/math_util.cc',
  'base/mimetypes.cc',
  'base/missing/strtod.cc',
  'base/referent.cc',
  'base/string_util.cc',
  'base/time_util.cc',
  'base/uri_parser.cc',
  'base/version.cc',
  'base/xml_element.cc',
  'base/xml_file.cc',
  'base/xml_namespaces.cc',
  'base/xmlns.cc',
  'base/zip_file.cc',
  'convenience/atom_util.cc',
  'convenience/convenience.cc',
  'convenience/csv_file.cc',
  'convenience/csv_parser.cc',
  'convenience/element_counter.cc',
  'convenience/feature_list.cc',
  'convenience/google_doc_list.cc',
  'convenience/google_maps_data.cc',
  'convenience/google_picasa_web.cc',
  'convenience/google_spreadsheets.cc',
  'convenience/gpx_trk_pt_handler.cc',
  'convenience/http_client.cc',
  'convenience/kml_feature_list_saver.cc',
  'convenience/kmz_check_links.cc',
  'dom/abstractlatlonbox.cc',
  'dom/abstractview.cc',
  'dom/atom.cc',
  'dom/balloonstyle.cc',
  'dom/colorstyle.cc',
  'dom/container.cc',
  'dom/document.cc',
  'dom/element.cc',
  'dom/extendeddata.cc',
  'dom/feature.cc',
  'dom/folder.cc',
  'dom/geometry.cc',
  'dom/gx_timeprimitive.cc',
  'dom/gx_tour.cc',
  'dom/hotspot.cc',
  'dom/iconstyle.cc',
  'dom/kml_cast.cc',
  'dom/kml.cc',
  'dom/kml_factory.cc',
  'dom/kml_handler.cc',
  'dom/kml_handler_ns.cc',
  'dom/labelstyle.cc',
  'dom/linestyle.cc',
  'dom/link.cc',
  'dom/liststyle.cc',
  'dom/model.cc',
  'dom/networklink.cc',
  'dom/networklinkcontrol.cc',
  'dom/object.cc',
  'dom/overlay.cc',
  'dom/parser.cc',
  'dom/parser_observer.cc',
  'dom/placemark.cc',
  'dom/polystyle.cc',
  'dom/region.cc',
  'dom/schema.cc',
  'dom/serializer.cc',
  'dom/snippet.cc',
  'dom/stats_serializer.cc',
  'dom/style.cc',
  'dom/stylemap.cc',
  'dom/styleselector.cc',
  'dom/substyle.cc',
  'dom/timeprimitive.cc',
  'dom/vec2.cc',
  'dom/visitor.cc',
  'dom/visitor_driver.cc',
  'dom/xal.cc',
  'dom/xml_serializer.cc',
  'dom/xsd.cc',
  'engine/bbox.cc',
  'engine/clone.cc',
  'engine/entity_mapper.cc',
  'engine/feature_balloon.cc',
  'engine/feature_view.cc',
  'engine/feature_visitor.cc',
  'engine/find.cc',
  'engine/find_xml_namespaces.cc',
  'engine/get_link_parents.cc',
  'engine/get_links.cc',
  'engine/href.cc',
  'engine/id_mapper.cc',
  'engine/kml_cache.cc',
  'engine/kml_file.cc',
  'engine/kml_stream.cc',
  'engine/kml_uri.cc',
  'engine/kmz_cache.cc',
  'engine/kmz_file.cc',
  'engine/link_util.cc',
  'engine/location_util.cc',
  'engine/merge.cc',
  'engine/object_id_parser_observer.cc',
  'engine/old_schema_parser_observer.cc',
  'engine/parse_old_schema.cc',
  'engine/schema_parser_observer.cc',
  'engine/shared_style_parser_observer.cc',
  'engine/style_inliner.cc',
  'engine/style_merger.cc',
  'engine/style_resolver.cc',
  'engine/style_splitter.cc',
  'engine/update.cc',
  'engine/update_processor.cc',
  'regionator/feature_list_region_handler.cc',
  'regionator/regionator.cc',
  'regionator/regionator_util.cc',
  'regionator/region_handler.cc',
  'xsd/xsd_complex_type.cc',
  'xsd/xsd_element.cc',
  'xsd/xsd_file.cc',
  'xsd/xsd_handler.cc',
  'xsd/xsd_primitive_type.cc',
  'xsd/xsd_schema.cc',
  'xsd/xsd_simple_type.cc',
  'xsd/xsd_type.cc',
  'xsd/xsd_util.cc',
  'xsd/xst_parser.cc',
] + file_src


args = []
cc = meson.get_compiler('cpp')
if cc.get_id() == 'msvc'
  args += '-wd4251'
endif

libkml_lib = shared_library('kml',
  sources : src,
  name_prefix : 'lib',
  include_directories : kml_inc,
  dependencies : [uriparser_dep, minizip_dep, zlib_dep, bmcl_dep, expat_dep],
  cpp_args : ['-DBUILDING_LIBKML'] + args
)

libkml_dep = declare_dependency(
  link_with : libkml_lib,
  include_directories : include_directories('..', is_system : true),
)
