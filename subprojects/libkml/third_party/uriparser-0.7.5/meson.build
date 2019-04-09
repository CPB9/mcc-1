src = [
  'lib/UriCommon.c',
  'lib/UriCommon.h',
  'lib/UriCompare.c',
  'lib/UriEscape.c',
  'lib/UriFile.c',
  'lib/UriIp4.c',
  'lib/UriIp4Base.c',
  'lib/UriIp4Base.h',
  'lib/UriNormalize.c',
  'lib/UriNormalizeBase.c',
  'lib/UriNormalizeBase.h',
  'lib/UriParse.c',
  'lib/UriParseBase.c',
  'lib/UriParseBase.h',
  'lib/UriQuery.c',
  'lib/UriRecompose.c',
  'lib/UriResolve.c',
  'lib/UriShorten.c',
]

inc = include_directories('include')

uriparser_lib = static_library('uriparser',
  sources : src,
  include_directories : inc,
)

uriparser_dep = declare_dependency(
  link_with : uriparser_lib,
  include_directories : inc,
)
