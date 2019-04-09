config_h = configure_file(configuration : cfg, input : 'curl_config.h.meson', output : 'curl_config.h')

inc = include_directories('../include', '.')
dep_inc = include_directories('../include', is_system : true)

src = [
  'vauth/vauth.c', 'vauth/cleartext.c', 'vauth/cram.c',
  'vauth/digest.c', 'vauth/digest_sspi.c', 'vauth/krb5_gssapi.c',
  'vauth/krb5_sspi.c', 'vauth/ntlm.c', 'vauth/ntlm_sspi.c', 'vauth/oauth2.c',
  'vauth/spnego_gssapi.c', 'vauth/spnego_sspi.c',
  'vtls/openssl.c', 'vtls/gtls.c', 'vtls/vtls.c', 'vtls/nss.c',
  'vtls/polarssl.c', 'vtls/polarssl_threadlock.c', 'vtls/axtls.c',
  'vtls/cyassl.c', 'vtls/schannel.c', 'vtls/darwinssl.c', 'vtls/gskit.c',
  'vtls/mbedtls.c',
  'file.c', 'timeval.c', 'base64.c', 'hostip.c', 'progress.c', 'formdata.c',
  'cookie.c', 'http.c', 'sendf.c', 'ftp.c', 'url.c', 'dict.c', 'if2ip.c', 'speedcheck.c',
  'ldap.c', 'version.c', 'getenv.c', 'escape.c', 'mprintf.c', 'telnet.c', 'netrc.c',
  'getinfo.c', 'transfer.c', 'strcase.c', 'easy.c', 'security.c', 'curl_fnmatch.c',
  'fileinfo.c', 'ftplistparser.c', 'wildcard.c', 'krb5.c', 'memdebug.c', 'http_chunks.c',
  'strtok.c', 'connect.c', 'llist.c', 'hash.c', 'multi.c', 'content_encoding.c', 'share.c',
  'http_digest.c', 'md4.c', 'md5.c', 'http_negotiate.c', 'inet_pton.c', 'strtoofft.c',
  'strerror.c', 'amigaos.c', 'hostasyn.c', 'hostip4.c', 'hostip6.c', 'hostsyn.c',
  'inet_ntop.c', 'parsedate.c', 'select.c', 'tftp.c', 'splay.c', 'strdup.c', 'socks.c',
  'ssh.c', 'ssh-libssh.c', 'curl_addrinfo.c', 'socks_gssapi.c', 'socks_sspi.c',
  'curl_sspi.c', 'slist.c', 'nonblock.c', 'curl_memrchr.c', 'imap.c', 'pop3.c', 'smtp.c',
  'pingpong.c', 'rtsp.c', 'curl_threads.c', 'warnless.c', 'hmac.c', 'curl_rtmp.c',
  'openldap.c', 'curl_gethostname.c', 'gopher.c', 'idn_win32.c',
  'http_proxy.c', 'non-ascii.c', 'asyn-ares.c', 'asyn-thread.c', 'curl_gssapi.c',
  'http_ntlm.c', 'curl_ntlm_wb.c', 'curl_ntlm_core.c', 'curl_sasl.c', 'rand.c',
  'curl_multibyte.c', 'hostcheck.c', 'conncache.c', 'pipeline.c', 'dotdot.c',
  'x509asn1.c', 'http2.c', 'smb.c', 'curl_endian.c', 'curl_des.c', 'system_win32.c',
  'mime.c', 'sha256.c', 'setopt.c', 'curl_path.c', 'curl_ctype.c', 'curl_range.c',
]

curl_lib = shared_library('curl',
  name_prefix : 'lib',
  include_directories : inc,
  sources : src,
  c_args : cflags,
  dependencies : libs,
)

curl_dep = declare_dependency(
  link_with : curl_lib,
  include_directories : dep_inc,
)
