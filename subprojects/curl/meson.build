project('curl', 'c')

if host_machine.system() != 'windows' and host_machine.system() != 'linux'
  error('Only linux and windows supported')
endif

cc = meson.get_compiler('c')

checked_flags = [
  '-pedantic', 
  '-Wall', 
  '-W', 
  '-Wpointer-arith',
  '-Wwrite-strings',
  '-Wunused',
  '-Wshadow',
  '-Winline',
  '-Wnested-externs',
  '-Wmissing-declarations',
  '-Wmissing-prototypes',
  '-Wno-long-long',
  '-Wfloat-equal',
  '-Wno-multichar',
  '-Wsign-compare',
  '-Wundef',
  '-Wno-format-nonliteral',
  '-Wendif-labels',
  '-Wstrict-prototypes',
  '-Wdeclaration-after-statement',
  '-Wstrict-aliasing=3',
  '-Wcast-align',
  '-Wtype-limits',
  '-Wold-style-declaration',
  '-Wmissing-parameter-type',
  '-Wempty-body',
  '-Wclobbered',
  '-Wignored-qualifiers',
  '-Wconversion',
  '-Wno-sign-conversion',
  '-Wvla',
  '-Wdouble-promotion',
  '-Wno-system-headers'
]

cflags = []

if cc.get_id() == 'clang' or cc.get_id() == 'gcc'
  foreach flag : checked_flags
    if cc.has_argument(flag)
      cflags += flag
    endif
  endforeach
endif

cflags += '-DBUILDING_LIBCURL'

cfg = configuration_data()
cfg.set_quoted('OS', host_machine.system())

#TODO: add options
cfg.set('CURL_DISABLE_FTP', false)
cfg.set('CURL_DISABLE_LDAP', true)
cfg.set('CURL_DISABLE_LDAPS', true)
cfg.set('CURL_DISABLE_TELNET', true)
cfg.set('CURL_DISABLE_DICT', true)
cfg.set('CURL_DISABLE_FILE', true)
cfg.set('CURL_DISABLE_TFTP', true)
cfg.set('CURL_DISABLE_RTSP', true)
cfg.set('CURL_DISABLE_POP3', true)
cfg.set('CURL_DISABLE_IMAP', true)
cfg.set('CURL_DISABLE_SMTP', true)
cfg.set('CURL_DISABLE_GOPHER', true)

if host_machine.system() == 'windows'
  cfg.set('HAVE_LIBDL', false)
  cfg.set('HAVE_LIBUCB', false)
  cfg.set('HAVE_LIBSOCKET', false)
  cfg.set('NOT_NEED_LIBNSL', false)
  cfg.set('HAVE_LIBNSL', false)
  cfg.set('HAVE_GETHOSTNAME', true)
  cfg.set('HAVE_LIBZ', false)
  cfg.set('HAVE_LIBCRYPTO', false)
  cfg.set('HAVE_DLOPEN', false)
  cfg.set('HAVE_ALLOCA_H', false)
  cfg.set('HAVE_ARPA_INET_H', false)
  cfg.set('HAVE_DLFCN_H', false)
  cfg.set('HAVE_FCNTL_H', true)
  cfg.set('HAVE_INTTYPES_H', false)
  cfg.set('HAVE_IO_H', true)
  cfg.set('HAVE_MALLOC_H', true)
  cfg.set('HAVE_MEMORY_H', true)
  cfg.set('HAVE_NETDB_H', false)
  cfg.set('HAVE_NETINET_IF_ETHER_H', false)
  cfg.set('HAVE_NETINET_IN_H', false)
  cfg.set('HAVE_NET_IF_H', false)
  cfg.set('HAVE_PROCESS_H', true)
  cfg.set('HAVE_PWD_H', false)
  cfg.set('HAVE_SETJMP_H', true)
  cfg.set('HAVE_SGTTY_H', false)
  cfg.set('HAVE_SIGNAL_H', true)
  cfg.set('HAVE_SOCKIO_H', false)
  cfg.set('HAVE_STDINT_H', false)
  cfg.set('HAVE_STDLIB_H', true)
  cfg.set('HAVE_STRINGS_H', false)
  cfg.set('HAVE_STRING_H', true)
  cfg.set('HAVE_SYS_PARAM_H', false)
  cfg.set('HAVE_SYS_POLL_H', false)
  cfg.set('HAVE_SYS_SELECT_H', false)
  cfg.set('HAVE_SYS_SOCKET_H', false)
  cfg.set('HAVE_SYS_SOCKIO_H', false)
  cfg.set('HAVE_SYS_STAT_H', true)
  cfg.set('HAVE_SYS_TIME_H', false)
  cfg.set('HAVE_SYS_TYPES_H', true)
  cfg.set('HAVE_SYS_UTIME_H', true)
  cfg.set('HAVE_TERMIOS_H', false)
  cfg.set('HAVE_TERMIO_H', false)
  cfg.set('HAVE_TIME_H', true)
  cfg.set('HAVE_UNISTD_H', false)
  cfg.set('HAVE_UTIME_H', false)
  cfg.set('HAVE_X509_H', false)
  cfg.set('HAVE_ZLIB_H', false)
  cfg.set('HAVE_SIZEOF_LONG_DOUBLE', true)
  cfg.set('SIZEOF_LONG_DOUBLE', 8)
  cfg.set('HAVE_SOCKET', true)
  cfg.set('HAVE_POLL', false)
  cfg.set('HAVE_SELECT', true)
  cfg.set('HAVE_STRDUP', true)
  cfg.set('HAVE_STRSTR', true)
  cfg.set('HAVE_STRTOK_R', false)
  cfg.set('HAVE_STRFTIME', true)
  cfg.set('HAVE_UNAME', false)
  cfg.set('HAVE_STRCASECMP', false)
  cfg.set('HAVE_STRICMP', true)
  cfg.set('HAVE_STRCMPI', true)
  cfg.set('HAVE_GETHOSTBYADDR', true)
  cfg.set('HAVE_GETTIMEOFDAY', false)
  cfg.set('HAVE_INET_ADDR', true)
  cfg.set('HAVE_INET_NTOA', true)
  cfg.set('HAVE_INET_NTOA_R', false)
  cfg.set('HAVE_TCGETATTR', false)
  cfg.set('HAVE_TCSETATTR', false)
  cfg.set('HAVE_PERROR', true)
  cfg.set('HAVE_CLOSESOCKET', true)
  cfg.set('HAVE_SETVBUF', false)
  cfg.set('HAVE_SIGSETJMP', false)
  cfg.set('HAVE_GETPASS_R', false)
  cfg.set('HAVE_STRLCAT', false)
  cfg.set('HAVE_GETPWUID', false)
  cfg.set('HAVE_GETEUID', false)
  cfg.set('HAVE_UTIME', true)
  cfg.set('HAVE_RAND_EGD', false)
  cfg.set('HAVE_RAND_SCREEN', false)
  cfg.set('HAVE_RAND_STATUS', false)
  cfg.set('HAVE_SSL_GET_SHUTDOWN', false)
  cfg.set('HAVE_GMTIME_R', false)
  cfg.set('HAVE_LOCALTIME_R', false)
  cfg.set('HAVE_GETHOSTBYADDR_R', false)
  cfg.set('HAVE_GETHOSTBYNAME_R', false)
  cfg.set('HAVE_SIGNAL_FUNC', true)
  cfg.set('HAVE_SIGNAL_MACRO', false)
  cfg.set('HAVE_GETHOSTBYADDR_R_5', false)
  cfg.set('HAVE_GETHOSTBYADDR_R_5_REENTRANT', false)
  cfg.set('HAVE_GETHOSTBYADDR_R_7', false)
  cfg.set('HAVE_GETHOSTBYADDR_R_7_REENTRANT', false)
  cfg.set('HAVE_GETHOSTBYADDR_R_8', false)
  cfg.set('HAVE_GETHOSTBYADDR_R_8_REENTRANT', false)
  cfg.set('HAVE_GETHOSTBYNAME_R_3', false)
  cfg.set('HAVE_GETHOSTBYNAME_R_3_REENTRANT', false)
  cfg.set('HAVE_GETHOSTBYNAME_R_5', false)
  cfg.set('HAVE_GETHOSTBYNAME_R_5_REENTRANT', false)
  cfg.set('HAVE_GETHOSTBYNAME_R_6', false)
  cfg.set('HAVE_GETHOSTBYNAME_R_6_REENTRANT', false)
  cfg.set('TIME_WITH_SYS_TIME', false)
  cfg.set('HAVE_O_NONBLOCK', false)
  cfg.set('HAVE_IN_ADDR_T', false)
  cfg.set('HAVE_INET_NTOA_R_DECL', false)
  cfg.set('HAVE_INET_NTOA_R_DECL_REENTRANT', false)
  cfg.set('HAVE_GETADDRINFO', false)
  cfg.set('STDC_HEADERS', true)
  cfg.set('RETSIGTYPE_TEST', true)
  cfg.set('HAVE_SIGACTION', false)
  cfg.set('HAVE_MACRO_SIGSETJMP', false)
  
  cfg.set('HAVE_WINDOWS_H', true)
  cfg.set('HAVE_WINSOCK_H', true)
  cfg.set('HAVE_WINSOCK2_H', true)
  cfg.set('HAVE_WS2TCPIP_H', true)
  cfg.set('HAVE_ASSERT_H', true)
  cfg.set('HAVE_LOCALE_H', true)
  cfg.set('HAVE_STDIO_H', true)
  cfg.set('HAVE_ERRNO_H', true)
  cfg.set('HAVE_STDBOOL_H', true)
  cfg.set('HAVE_STDDEF_H', true)
  cfg.set('HAVE_POLL_H', false)
  cfg.set('HAVE_SSL_H', false)
  cfg.set('HAVE_KRB_H', false)
  cfg.set('HAVE_CRYPTO_H', false)
  cfg.set('HAVE_RSA_H', true)
  cfg.set('HAVE_ARPA_TFTP_H', true)
  cfg.set('HAVE_SYS_FILIO_H', false)
  cfg.set('HAVE_SYS_IOCTL_H', false)
  cfg.set('HAVE_SYS_UIO_H', false)
  cfg.set('HAVE_SYS_UN_H', false)
  cfg.set('HAVE_SYS_XATTR_H', false)
  cfg.set('HAVE_SYS_RESOURCE_H', false)
  cfg.set('HAVE_DES_H', false)
  cfg.set('HAVE_ERR_H', false)
  cfg.set('HAVE_IDN2_H', false)
  cfg.set('HAVE_IFADDRS_H', false)
  cfg.set('HAVE_LIBGEN_H', false)
  cfg.set('HAVE_PEM_H', false)
  cfg.set('HAVE_STROPTS_H', false)
  cfg.set('HAVE_SYS_UTSNAME_H', false)
  cfg.set('HAVE_NETINET_TCP_H', false)
 
  cfg.set('HAVE_GLIBC_STRERROR_R', false)
  cfg.set('HAVE_POSIX_STRERROR_R', false)
  cfg.set('HAVE_FCNTL_O_NONBLOCK', false)
  cfg.set('HAVE_IOCTLSOCKET', true)
  cfg.set('HAVE_IOCTLSOCKET_CAMEL', false)
  cfg.set('HAVE_IOCTLSOCKET_CAMEL_FIONBIO', true)
  cfg.set('HAVE_IOCTLSOCKET_FIONBIO', true)
  cfg.set('HAVE_IOCTL_FIONBIO', false)
  cfg.set('HAVE_IOCTL_SIOCGIFADDR', false)
  cfg.set('HAVE_SETSOCKOPT_SO_NONBLOCK', false)
  cfg.set('HAVE_SOCKADDR_IN6_SIN6_SCOPE_ID', false)
  cfg.set('HAVE_SOCKLEN_T', false)
  cfg.set('HAVE_BOOL_T', true)
  cfg.set('HAVE_FILE_OFFSET_BITS', false)
  cfg.set('HAVE_POLL_FINE', false)
  
  cfg.set('HAVE_BASENAME', false)
  cfg.set('HAVE_ALARM', false)
  cfg.set('HAVE_SIGNAL', false)
  cfg.set('HAVE_FCNTL', false)
  cfg.set('HAVE_IOCTL', false)
  cfg.set('HAVE_SETLOCALE', true)
  cfg.set('HAVE_STRNCMPI', false)
  cfg.set('HAVE_GETHOSTBYNAME', true)
  cfg.set('HAVE_STRTOLL', true)
  cfg.set('HAVE_SIGALRM', false)
  cfg.set('HAVE_STRERROR_R', false)
  cfg.set('HAVE_SIGINTERRUPT', false)
  cfg.set('HAVE_FORK', false)
  cfg.set('HAVE_FREEADDRINFO', true)
  cfg.set('HAVE_PIPE', false)
  cfg.set('HAVE_FREEIFADDRS', false)
  
  cfg.set('HAVE_FTRUNCATE', false)
  cfg.set('HAVE_GETPROTOBYNAME', true)
  cfg.set('HAVE_GETRLIMIT', false)
  cfg.set('HAVE_SETMODE', true)
  cfg.set('HAVE_SETRLIMIT', false)
  cfg.set('HAVE_SETSOCKOPT', true)
  cfg.set('HAVE_MACH_ABSOLUTE_TIME', false)
  cfg.set('HAVE_FSETXATTR', false)
  cfg.set('HAVE_FSETXATTR5', false)
  cfg.set('HAVE_FSETXATTR6', false)
  cfg.set('HAVE_INET_PTON', true)
endif

libs = []

if host_machine.system() == 'windows'
  lib_defs = [
    'ws2_32',
    'winmm',
  ]
else
  lib_defs = [
    'dl',
    'socket',
    'bind',
    'bnetapi',
  ]
endif


foreach ld : lib_defs
  l = cc.find_library(ld, required : false)
  if l.found()
    libs += l
    cfg.set('HAVE_' + ld.to_upper(), true)
  endif
endforeach

if host_machine.system() == 'windows'
    cfg.set('USE_UNIX_SOCKETS', false)
else
  if cc.has_member('struct sockaddr_un', 'sun_path', prefix : '#include<sys/un.h>')
    cfg.set('USE_UNIX_SOCKETS', true)
  endif
endif

inc_files = [
  'stdio.h',
  'inttypes.h',
  'sys/filio.h',
  'sys/ioctl.h',
  'sys/param.h',
  'sys/poll.h',
  'sys/resource.h',
  'sys/select.h',
  'sys/socket.h',
  'sys/sockio.h',
  'sys/stat.h',
  'sys/time.h',
  'sys/types.h',
  'sys/uio.h',
  'sys/un.h',
  'sys/utime.h',
  'sys/xattr.h',
  'alloca.h',
  'arpa/inet.h',
  'arpa/tftp.h',
  'assert.h',
  'crypto.h',
  'des.h',
  'err.h',
  'errno.h',
  'fcntl.h',
  'idn2.h',
  'ifaddrs.h',
  'io.h',
  'krb.h',
  'libgen.h',
  'locale.h',
  'net/if.h',
  'netdb.h',
  'netinet/in.h',
  'netinet/tcp.h',
  'pem.h',
  'poll.h',
  'pwd.h',
  'rsa.h',
  'setjmp.h',
  'sgtty.h',
  'signal.h',
  'ssl.h',
  'stdbool.h',
  'stdint.h',
  'stdio.h',
  'stdlib.h',
  'string.h',
  'strings.h',
  'stropts.h',
  'termio.h',
  'termios.h',
  'time.h',
  'unistd.h',
  'utime.h',
  'x509.h',
  'process.h',
  'stddef.h',
  'dlfcn.h',
  'malloc.h',
  'memory.h',
  'netinet/if_ether.h',
  'stdint.h',
  'sockio.h',
  'sys/utsname.h',
]

all_inc_prefix = ''
if host_machine.system() == 'windows'
  inc_files += ['windows.h', 'winsock.h', 'ws2tcpip.h', 'winsock2.h']
  all_inc_prefix += '#define _WINSOCKAPI_\n'
  if cfg.has('HAVE_LIBWS2_32')
    all_inc_prefix += '#pragma comment(lib, "Ws2_32.lib")\n'
  endif
  ssl_dep = cc.find_library('crypt32', required : true)
  cflags += '-DSECURITY_WIN32'
  cfg.set('USE_SCHANNEL', true)
  cfg.set('USE_WINDOWS_SSPI', true)
else
  ssl_dep = dependency('openssl', required : true)
  inc_files += ['openssl/crypto.h', 'openssl/engine.h', 'openssl/err.h', 'openssl/pem.h',
                'openssl/rsa.h', 'openssl/ssl.h', 'openssl/x509.h', 'openssl/rand.h']
  cfg.set('USE_OPENSSL', true)
  cfg.set('HAVE_LIBCRYPTO', true)
  cfg.set('HAVE_LIBSSL', true)
endif

libs += ssl_dep

foreach i : inc_files
  def = 'HAVE_' + i.to_upper().underscorify()
  if not cfg.has(def)
    if cc.has_header(i, dependencies : ssl_dep)
      cfg.set(def, true)
      all_inc_prefix += '#include <' + i + '>\n'
    endif
  endif
endforeach

cfg2 = cfg
configure_file(configuration : cfg2, input : 'lib/curl_config.h.meson', output : 'temp_config.h')

size_defs = [
  'size_t',
  'ssize_t',
  'long long',
  'long',
  'short',
  'int',
  '__int64',
  'long double',
  'time_t',
]

foreach s : size_defs
  def = 'HAVE_SIZEOF_' + '_'.join(s.split()).to_upper()
  if not cfg.has(def)
    prefix = '#include <time.h>\n#include <stddef.h>\n'
    if host_machine.system() == 'linux'
        prefix += '#include <unistd.h>\n'
    endif
    size = cc.sizeof(s, prefix : prefix)
    if size > 0
      cfg.set(def, true)
      cfg.set('SIZEOF_' + s.to_upper(), size)
    endif
  endif
endforeach

if not cfg.has('HAVE_SIZEOF_SSIZE_T')
  if cfg.get('SIZEOF_LONG') == cfg.get('SIZEOF_SIZE_T')
    cfg.set('ssize_t', 'long')
  elif cfg.has('SIZEOF___INT64') and (cfg.get('SIZEOF___INT64') == cfg.get('SIZEOF_SIZE_T'))
    cfg.set('ssize_t', '__int64')
  endif
endif

if cfg.has('HAVE_SIZEOF_LONG_LONG')
  cfg.set('HAVE_LONGLONG', true)
  cfg.set('HAVE_LL', true)
endif

sym_exists_template = '''
int main(int argc, char** argv)
{
(void)argv;
#ifndef @0@
    return ((int*)(&@0@))[argc];
#else
    (void)argc;
    return 0;
#endif
}
'''

syms = [
  'basename',
  'socket',
  'select',
  'strdup',
  'strstr',
  'strtok_r',
  'strftime',
  'uname',
  'strcasecmp',
  'stricmp',
  'strcmpi',
  'strncmpi',
  'alarm',
  'gethostbyaddr',
  'gethostbyaddr_r',
  'gettimeofday',
  'inet_addr',
  'inet_ntoa',
  'inet_ntoa_r',
  'tcsetattr',
  'tcgetattr',
  'perror',
  'closesocket',
  'setvbuf',
  'sigsetjmp',
  'getpass_r',
  'strlcat',
  'getpwuid',
  'geteuid',
  'utime',
  'gmtime_r',
  'localtime_r',
  'gethostbyname',
  'gethostbyname_r',
  'signal',
  'SIGALRM',
  'uname',
  'strtoll',
  'strerror_r',
  'siginterrupt',
  'perror',
  'fork',
  'getaddrinfo',
  'freeaddrinfo',
  'freeifaddrs',
  'pipe',
  'ftruncate',
  'getprotobyname',
  'getrlimit',
  'setlocale',
  'setmode',
  'setrlimit',
  'fcntl',
  'ioctl',
  'setsockopt',
  'mach_absolute_time',
  'fsetxattr',
  'inet_pton',
  'RAND_status',
  'RAND_screen',
  'RAND_egd',
  'SSL_get_shutdown',
]

if host_machine.system() != 'darwin'
  syms += 'poll'
endif

foreach s : syms
  def = 'HAVE_' + s.to_upper()
  if not cfg.has(def)
    prog = all_inc_prefix + sym_exists_template.format(s)
    if cc.compiles(prog, name : s)
      cfg.set(def, true)
    endif
  endif
endforeach

if cfg.has('HAVE_SIGNAL_FUNC') and cfg.has('HAVE_SIGNAL_MACRO')
  cfg.set('HAVE_SIGNAL', true)
  def = 'HAVE_SIGACTION'
  if not cfg.has(def)
    prog = '#include "signal.h"' + sym_exists_template.format('sigaction')
    if cc.compiles(prog, name : 'HAVE_SIGACTION')
      cfg.set(def, true)
    endif
  endif
endif

if not cfg.has('HAVE_STRNCMPI')
  cfg.set('HAVE_STRCMPI', true)
endif

if not cfg.has('HAVE_SIGSETJMP')
  def1 = 'HAVE_MACRO_SIGSETJMP'
  def2 = 'HAVE_SIGSETJMP'
  if cfg.has('HAVE_SETJMP_H') and not (cfg.has(def1) or cfg.has(def2))
    prog = '#include "setjmp.h"' + sym_exists_template.format('sigsetjmp')
    if cc.compiles(prog, name : 'HAVE_SIGSETJMP')
      cfg.set(def1, true)
      cfg.set(def2, true)
    endif
  endif
endif

if host_machine.system() == 'windows'
    cflags += '-D_WIN32_WINNT=0x0600'
endif

curl_tests = [
  'HAVE_FCNTL_O_NONBLOCK',
  'HAVE_IOCTLSOCKET',
  'HAVE_IOCTLSOCKET_CAMEL',
  'HAVE_IOCTLSOCKET_CAMEL_FIONBIO',
  'HAVE_IOCTLSOCKET_FIONBIO',
  'HAVE_IOCTL_FIONBIO',
  'HAVE_IOCTL_SIOCGIFADDR',
  'HAVE_SETSOCKOPT_SO_NONBLOCK',
  'HAVE_SOCKADDR_IN6_SIN6_SCOPE_ID',
  'TIME_WITH_SYS_TIME',
  'HAVE_O_NONBLOCK',
  'HAVE_GETHOSTBYADDR_R_5',
  'HAVE_GETHOSTBYADDR_R_7',
  'HAVE_GETHOSTBYADDR_R_8',
  'HAVE_GETHOSTBYADDR_R_5_REENTRANT',
  'HAVE_GETHOSTBYADDR_R_7_REENTRANT',
  'HAVE_GETHOSTBYADDR_R_8_REENTRANT',
  'HAVE_GETHOSTBYNAME_R_3',
  'HAVE_GETHOSTBYNAME_R_5',
  'HAVE_GETHOSTBYNAME_R_6',
  'HAVE_GETHOSTBYNAME_R_3_REENTRANT',
  'HAVE_GETHOSTBYNAME_R_5_REENTRANT',
  'HAVE_GETHOSTBYNAME_R_6_REENTRANT',
  'HAVE_SOCKLEN_T',
  'HAVE_IN_ADDR_T',
  'HAVE_BOOL_T',
  'STDC_HEADERS',
  'RETSIGTYPE_TEST',
  'HAVE_INET_NTOA_R_DECL',
  'HAVE_INET_NTOA_R_DECL_REENTRANT',
  'HAVE_GETADDRINFO',
  'HAVE_FILE_OFFSET_BITS',
]

if cfg.has('HAVE_FSETXATTR')
  curl_tests += ['HAVE_FSETXATTR5']
  curl_tests += ['HAVE_FSETXATTR6']
endif

test_code = '#include "' + meson.current_build_dir() + '/temp_config.h"\n'
test_code += '#include "' + meson.current_source_dir() + '/CMake/CurlTests.c"\n'

foreach t : curl_tests
  if not cfg.has(t)
    prefix = '#define ' + t + '\n'
    if cc.compiles(prefix + test_code, name : t)
      cfg.set(t, true)
    endif
  endif
endforeach

file_offset_prefix = ''

if cfg.has('HAVE_FILE_OFFSET_BITS')
  cfg.set('_FILE_OFFSET_BITS', 64)
  cflags += '-D_FILE_OFFSET_BITS=64'
  file_offset_prefix = '#define _FILE_OFFSET_BITS 64\n#include "sys/types.h"'
endif

def = 'SIZEOF_OFF_T'
if not cfg.has(def)
  size = cc.sizeof('off_t', prefix : file_offset_prefix)
  cfg.set(def, size)
endif

def = 'SIZEOF_CURL_OFF_T'
if not cfg.has(def)
  curl_off_t_prefix = file_offset_prefix + '\n#include "' + meson.current_source_dir() + '/include/curl/system.h"'
  cfg.set('SIZEOF_CURL_OFF_T', cc.sizeof('curl_off_t', prefix : curl_off_t_prefix))
endif

curl_run_tests = [
  'HAVE_GLIBC_STRERROR_R',
  'HAVE_POSIX_STRERROR_R',
]

foreach t : curl_run_tests
  if not cfg.has(t)
    prefix = '#define ' + t + '\n'
    rv = cc.run(prefix + test_code, name : t)
    if rv.returncode() == 0
      cfg.set(t, true)
    endif
  endif
endforeach

need_reentrant = false

foreach t : [
    'HAVE_GETHOSTBYADDR_R_5',
    'HAVE_GETHOSTBYADDR_R_7',
    'HAVE_GETHOSTBYADDR_R_8',
    'HAVE_GETHOSTBYNAME_R_3',
    'HAVE_GETHOSTBYNAME_R_5',
    'HAVE_GETHOSTBYNAME_R_6',
    'HAVE_INET_NTOA_R_DECL_REENTRANT',
  ]
  if not cfg.has(t)
    if cfg.has(t + '_REENTRANT')
      need_reentrant = false
    endif
  endif
endforeach

if need_reentrant
  foreach t : [
      'HAVE_GETHOSTBYADDR_R_5',
      'HAVE_GETHOSTBYADDR_R_7',
      'HAVE_GETHOSTBYADDR_R_8',
      'HAVE_GETHOSTBYNAME_R_3',
      'HAVE_GETHOSTBYNAME_R_5',
      'HAVE_GETHOSTBYNAME_R_6',
    ]


    if not cfg.has(t)
      cfg.set(t, false)
    endif
    if cfg.has(t + '_REENTRANT')
      cfg.set(t, true)
    endif
  endforeach
endif

if cfg.has('HAVE_INET_NTOA_R_DECL_REENTRANT')
  cfg.set('HAVE_INET_NTOA_R_DECL', true)
  need_reentrant = true
endif

if need_reentrant
  cfg.set('NEED_REENTRANT', true)
endif

if not cfg.has('HAVE_IN_ADDR_T')
  cfg.set('in_addr_t', 'unsigned long')
endif

if not(cfg.has('HAVE_FIONBIO') or cfg.has('HAVE_IOCTLSOCKET') or cfg.has('HAVE_IOCTLSOCKET_CASE') or cfg.has('HAVE_O_NONBLOCK'))
  cfg.set('HAVE_DISABLED_NONBLOCKING', true)
endif

if cfg.has('RETSIGTYPE_TEST')
  cfg.set('RETSIGTYPE', 'void')
else
  cfg.set('RETSIGTYPE', 'int')
endif

sock_test = ''
if host_machine.system() == 'windows'
  if cfg.has('HAVE_WINSOCK2_H')
    sock_test += '#include "winsock2.h"\n'
  endif
  if cfg.has('HAVE_WINDOWS_H')
    sock_test += '#include "windows.h"\n'
  endif
  if cfg.has('HAVE_WINSOCK_H')
    sock_test += '#include "winsock.h"\n'
  endif
  if cfg.has('HAVE_LIBWS2_32')
    sock_test += ' #pragma comment(lib, "Ws2_32.lib")\n'
  endif
else
  if cfg.has('HAVE_SYS_TYPES_H')
    sock_test += '#include "sys/types.h"\n'
  endif
  if cfg.has('HAVE_SYS_SOCKET_H')
    sock_test += '#include "sys/socket.h"\n'
  endif
endif

cfg.set('HAVE_RECV', true)
cfg.set('HAVE_SEND', true)
cfg.set('HAVE_STRUCT_TIMEVAL', true)
if host_machine.system() == 'windows'
  cfg.set('RECV_TYPE_RETV', 'int')
  cfg.set('RECV_TYPE_ARG1', 'SOCKET')
  cfg.set('RECV_TYPE_ARG2', 'char*')
  cfg.set('RECV_TYPE_ARG3', 'int')
  cfg.set('RECV_TYPE_ARG4', 'int')
  cfg.set('SEND_TYPE_RETV', 'int')
  cfg.set('SEND_TYPE_ARG1', 'SOCKET')
  cfg.set('SEND_QUAL_ARG2', 'const')
  cfg.set('SEND_TYPE_ARG2', 'char*')
  cfg.set('SEND_TYPE_ARG3', 'int')
  cfg.set('SEND_TYPE_ARG4', 'int')
else
  cfg.set('RECV_TYPE_RETV', 'ssize_t')
  cfg.set('RECV_TYPE_ARG1', 'int')
  cfg.set('RECV_TYPE_ARG2', 'void*')
  cfg.set('RECV_TYPE_ARG3', 'size_t')
  cfg.set('RECV_TYPE_ARG4', 'int')
  cfg.set('SEND_TYPE_RETV', 'ssize_t')
  cfg.set('SEND_TYPE_ARG1', 'int')
  cfg.set('SEND_QUAL_ARG2', 'const')
  cfg.set('SEND_TYPE_ARG2', 'void*')
  cfg.set('SEND_TYPE_ARG3', 'size_t')
  cfg.set('SEND_TYPE_ARG4', 'int')
endif

sys_poll_prefix = sock_test
if cfg.has('HAVE_SYS_POLL_H')
  sys_poll_prefix = '#include <sys/poll.h>\n'
endif

sys_poll_test = '''
  int main(void) {
    return poll((void *)0, 0, 10 /*ms*/);
  }
'''

def = 'HAVE_POLL_FINE'
if not cfg.has(def)
  sys_poll_runs = cc.run(sys_poll_prefix + sys_poll_test, name : 'HAVE_POLL_FINE')
  if sys_poll_runs.returncode() == 0
    cfg.set(def, true)
  endif
endif

cfg.set('HAVE_SIG_ATOMIC_T', true)
signal_prefix = ''
if cfg.has('HAVE_SIGNAL_H')
  signal_prefix = '#include <signal.h>\n'
endif

def = 'HAVE_SIZEOF_SIG_ATOMIC_T'
if not cfg.has(def)
  sig_atomic_size = cc.sizeof('sig_atomic_t', prefix : signal_prefix)
  if sig_atomic_size > 0
    cfg.set('SIZEOF_SIG_ATOMIC_T', sig_atomic_size)
    cfg.set(def, true)
    test = '''
       int main(void) {
         static volatile sig_atomic_t dummy = 0;
         (void)dummy;
         return 0;
       }'''

    if cc.compiles(signal_prefix + test, name : 'HAVE_SIG_ATOMIC_T_NOT_VOLATILE')
      cfg.set('HAVE_SIG_ATOMIC_T_NOT_VOLATILE', true)
    else
      cfg.set('HAVE_SIG_ATOMIC_T_VOLATILE', true)
    endif
  endif
endif

def = 'HAVE_SIZEOF_STRUCT_SOCKADDR_STORAGE'
if not cfg.has(def)
  sockaddr_storage_size = cc.sizeof('struct sockaddr_storage', prefix : sock_test)
  if sockaddr_storage_size > 0
    cfg.set(def, true)
    cfg.set('HAVE_STRUCT_SOCKADDR_STORAGE', true)
    cfg.set('SIZEOF_STRUCT_SOCKADDR_STORAGE', sockaddr_storage_size)
  endif
endif

cflags += '-DHAVE_CONFIG_H'

subdir('lib')
