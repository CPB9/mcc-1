rc = [
  'map.qrc',
]

moc_headers = [
  'CacheStackModel.h',
  'CacheStackView.h',
  'CurlEasy.h',
  'CurlMulti.h',
  'FileCache.h',
  'Layer.h',
  'LayerGroup.h',
  'LayerModel.h',
  'LayerWidget.h',
  'MapLayer.h',
  'MapRect.h',
  'MapWidget.h',
  'MapWidgetAnimator.h',
  'MultiselectLayer.h',
  'OmcfCacheWidget.h',
  'SimpleFlagLayer.h',
  'TileLoader.h',
  'UserWidget.h',
  'mapwidgets/AbstractPropertiesWidget.h',
  'mapwidgets/ListViewDelegate.h',
]

processed = qt5_mod.preprocess(
  moc_headers : moc_headers,
  include_directories : mcc_inc,
  qresources : rc,
)

src = [
  'CachePlugin.cpp',
  'CacheStackModel.cpp',
  'CacheStackView.cpp',
  'CurlDownloader.cpp',
  'CurlEasy.cpp',
  'CurlMulti.cpp',
  'DiskCache.cpp',
  'FileCache.cpp',
  'FlagRenderer.cpp',
  'FollowAnimator.cpp',
  'Layer.cpp',
  'LayerGroup.cpp',
  'LayerModel.cpp',
  'LayerPlugin.cpp',
  'LayerWidget.cpp',
  'MapLayer.cpp',
  'MapRect.cpp',
  'MapWidget.cpp',
  'MapWidgetAnimator.cpp',
  'MapWidgetPlugin.cpp',
  'MemoryCache.cpp',
  'MultiselectLayer.cpp',
  'OmcfCache.cpp',
  'OmcfCacheWidget.cpp',
  'OnlineCache.cpp',
  'OsmBasicCache.cpp',
  'SimpleFlagLayer.cpp',
  'StackCache.cpp',
  'TileLoader.cpp',
  'UserWidget.cpp',
  'drawables/BiMarker.cpp',
  'drawables/Flag.cpp',
  'drawables/Label.cpp',
  'drawables/LostSignalMarker.cpp',
  'drawables/Marker.cpp',
  'drawables/MarkerBase.cpp',
  'drawables/Point.cpp',
  'drawables/RulerLabel.cpp',
  'drawables/WithPosition.cpp',
  'drawables/WithPosition.h',
  'mapwidgets/AbstractPropertiesWidget.cpp',
  'mapwidgets/ListViewDelegate.cpp',
  'mapwidgets/Methods.cpp',
]

mcc_map_lib = shared_library('mcc-map',
  name_prefix : 'lib',
  sources : src + processed,
  link_with : [],
  include_directories : mcc_inc,
  dependencies : [qt5_core_dep, qt5_gui_dep, qt5_widgets_dep, qt5_printsupport_dep, qt5_opengl_dep, bmcl_dep, fmt_dep, curl_dep, mcc_geo_dep, mcc_msg_dep, thread_dep, mcc_plugin_dep, mcc_res_dep, mcc_ui_dep],
  cpp_args : '-DBUILDING_MCC_MAP',
)

all_mcc_libs += mcc_map_lib

all_mcc_tools += executable('omcf-creator',
  sources : 'Main.cpp',
  link_with : [mcc_map_lib],
  include_directories : mcc_inc,
  dependencies : [qt5_core_dep, qt5_gui_dep, qt5_widgets_dep, bmcl_dep, mcc_geo_dep],
)
