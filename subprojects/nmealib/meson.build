project('nmealib', 'c', default_options : 'c_std=c99')

headers = [
  'include/nmea/config.h',
  'include/nmea/context.h',
  'include/nmea/generate.h',
  'include/nmea/generator.h',
  'include/nmea/gmath.h',
  'include/nmea/info.h',
  'include/nmea/nmea.h',
  'include/nmea/parse.h',
  'include/nmea/parser.h',
  'include/nmea/sentence.h',
  'include/nmea/time.h',
  'include/nmea/tok.h',
  'include/nmea/units.h',
  'include/nmea/strtod.h',
]

src = [
  'src/context.c',
  'src/generate.c',
  'src/generator.c',
  'src/gmath.c',
  'src/info.c',
  'src/parse.c',
  'src/parser.c',
  'src/sentence.c',
  'src/time.c',
  'src/tok.c',
  'src/strtod.c',
]

inc = include_directories('include')

nmea_lib = static_library('nmea',
  sources : src + headers,
  include_directories : inc,
)

nmea_dep = declare_dependency(
  link_with : nmea_lib,
  include_directories : inc,
)

