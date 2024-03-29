# Copyright © 2018 Dylan Baker

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

config = configuration_data()
cc = meson.get_compiler('c')

# Define how much context to retain around the current parse point.
config.set('XML_CONTEXT_BYTES', 1024)

if cc.has_function('arc4random_buf')
  config.set('HAVE_ARC4RANDOM_BUF', true)
elif cc.has_function('arc4random')
  config.set('HAVE_ARC4RANDOM', true)
endif

config.set('XML_DTD', true)
config.set('XML_NS', true)

foreach h : ['dlfcn', 'fcntl', 'inttypes', 'memory', 'stdint', 'stdlib',
             'strings', 'string', 'sys/stat', 'sys/types', 'unistd']
  if cc.has_header(h + '.h')
    config.set('HAVE_@0@_H'.format(h.underscorify().to_upper()), true)
  endif
endforeach

foreach f : ['getpagesize', 'bcopy', 'mmap', 'getrandom', 'memmove']
  if cc.has_function(f)
    config.set('HAVE_@0@'.format(f.to_upper()), true)
  endif
endforeach

if host_machine.endian() == 'little'
  config.set('BYTEORDER', 1234)
else
  config.set('BYTEORDER', 4321)
endif


if not (config.get('HAVE_SYS_TYPES_H', false) and
        cc.has_header_symbol('sys/types.h', 'off_t'))
  config.set('off_t', 'long')
endif
if not (config.get('HAVE_SYS_TYPES_H', false) and
        cc.has_header_symbol('sys/types.h', 'size_t'))
  config.set('size_t', 'unsigned')
endif

if cc.compiles('''
      #include <stdlib.h>       /* for NULL */
      #include <unistd.h>       /* for syscall */
      #include <sys/syscall.h>  /* for SYS_getrandom */
      int main() {
        syscall(SYS_getrandom, NULL, 0, 0);
        return 0;
      }''',
      name : 'SYS_getrandom',
    )
  config.set('HAVE_SYSCALL_GETRANDOM', true)
endif

config_h = configure_file(
  configuration : config,
  output : 'expat_config.h',
)

args = ['-DHAVE_EXPAT_CONFIG_H']

if cc.get_id() == 'msvc'
  args += '-DXML_STATIC'
endif
if not ['windows', 'cygwin'].contains(host_machine.system())
  args += '-DXML_DEV_URANDOM'
endif
dep_args = args
if cc.get_id() == 'msvc'
  args += ['-D_CRT_SECURE_NO_WARNINGS', '-wd4996']
endif
if cc.has_argument('-fno-strict-aliasing')
  args += '-fno-strict-aliasing'
endif

soversion = host_machine.system() != 'windows' ? '1' : ''

libexpat = static_library(
  'expat',
  [
    files(
      'lib/loadlibrary.c', 'lib/xmlparse.c', 'lib/xmlrole.c', 'lib/xmltok.c',
      'lib/xmltok_impl.c', 'lib/xmltok_ns.c',
    ),
    config_h,
  ],
  include_directories : include_directories('lib'),
  version : '1.6.7',
  soversion : soversion,
  c_args : args,
)

expat_dep = declare_dependency(
  sources : config_h,
  link_with : libexpat,
  include_directories : include_directories('lib'),
  compile_args : dep_args,
)

# TODO: tools, examples, tests, docs
# These are probably not necessary for a wrap, but someone might have use for
# them
