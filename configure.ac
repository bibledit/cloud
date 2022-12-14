AC_INIT([bibledit],[5.0.994],[http://bibledit.org])
AM_INIT_AUTOMAKE([tar-ustar subdir-objects])
AC_CANONICAL_BUILD
ac_default_prefix=/usr

AC_PROG_CC
AC_PROG_CXX
AC_LANG([C++])
AX_CXX_COMPILE_STDCXX(17,[noext],[mandatory])
AC_PROG_RANLIB
AC_CONFIG_HEADERS([config.h])


AC_PATH_PROG(GZIP_PATH, gzip, no)
if test x$GZIP_PATH = xno; then
  AC_MSG_ERROR(Program "gzip" is needed. Install this first.)
fi


AC_PATH_PROG(GUNZIP_PATH, gunzip, no)
if test x$GUNZIP_PATH = xno; then
  AC_MSG_ERROR(Program "gunzip" is needed. Install this first.)
fi


AC_PATH_PROG(FIND_TAR, tar, no)
if test x$FIND_TAR = xno; then
  AC_MSG_ERROR(Program "tar" is needed. Install this first.)
fi


AC_PATH_PROG(ZIP_PATH, zip, no)
if test x$ZIP_PATH = xno; then
  AC_MSG_ERROR(Program "zip" is needed. Install this first.)
fi


AC_PATH_PROG(UNZIP_PATH, unzip, no)
if test x$UNZIP_PATH = xno; then
  AC_MSG_ERROR(Program "unzip" is needed. Install this first.)
fi


AC_PATH_PROG(INSTALLMGR_PATH, installmgr, no)
AC_PATH_PROG(DIATHEKE_PATH, diatheke, no)


PKG_PROG_PKG_CONFIG
# https://lintian.debian.org/tags/autotools-pkg-config-macro-not-cross-compilation-safe.html
if test x$PKG_CONFIG = x; then
  AC_MSG_ERROR(Program "pkg-config" is needed. Install this first.)
fi

PKG_CHECK_MODULES(CURL, libcurl >= 7.20.0,,AC_MSG_ERROR(libcurl development version >= 7.20.0 is needed.))
AC_SUBST(CURL_CFLAGS)
AC_SUBST(CURL_LIBS)
AC_CHECK_HEADER(curl/curl.h, , AC_MSG_ERROR([Header file curl/curl.h was not found. Usually this is in libcurl4-openssl-dev or libcurl4-gnutls-dev.]))

PKG_CHECK_MODULES(OPENSSL, openssl >= 0.9.0,,AC_MSG_ERROR(openssl development version >= 0.9.0 is needed.))
AC_SUBST(OPENSSL_CFLAGS)
AC_SUBST(OPENSSL_LIBS)

PKG_CHECK_MODULES(XML2, libxml-2.0 >= 2.9.0,,AC_MSG_ERROR(GNOME XML2 library development version >= 2.9.0 is needed.))
AC_SUBST(XML2_CFLAGS)
AC_SUBST(XML2_LIBS)

# linux PKG_CHECK_MODULES(GTK, gtk+-3.0,,AC_MSG_ERROR(Gtk3 development version is needed.))
# linux AC_SUBST(GTK_LIBS)
# linux AC_SUBST(GTK_CFLAGS)

# linux PKG_CHECK_MODULES([WEBKIT2GTK], [webkit2gtk-4.1],, [
# linux   PKG_CHECK_MODULES([WEBKIT2GTK], [webkit2gtk-4.0],, [
# linux     PKG_CHECK_MODULES([WEBKIT2GTK], [webkit2gtk-3.0],,AC_MSG_ERROR(Webkit2Gtk development version is needed.))
# linux   ])
# linux ])
# linux AC_SUBST(WEBKIT2GTK_LIBS)
# linux AC_SUBST(WEBKIT2GTK_CFLAGS)

AC_CHECK_LIB([mbedtls], [mbedtls_ssl_init])
AM_CONDITIONAL([HAVE_LIBMBEDTLS], [test "$ac_cv_lib_mbedtls_ssl_init" = yes])

# Tag1 (leave in place)
# Tag2 (leave in place)
# Tag3 (leave in place)
# Tag4 (leave in place)
# Tag5 (leave in place)
# Tag6 (leave in place)
# Tag7 (leave in place)
# Tag8 (leave in place)
# Tag9 (leave in place)
# TagA (leave in place)
# TagB (leave in place)
# TagC (leave in place)
# TagD (leave in place)
# TagE (leave in place)
# TagF (leave in place)
# TagG (leave in place)


AC_CHECK_HEADER(pthread.h,AC_DEFINE([HAVE_PTHREAD],[1],[Define whether pthread.h is present]),)

AC_CHECK_HEADER(libproc.h,AC_DEFINE([HAVE_LIBPROC],[1],[Define whether libproc.h is present]),)

AC_CHECK_HEADER(execinfo.h,AC_DEFINE([HAVE_EXECINFO],[1],[Define whether execinfo.h is present]),)

AC_CHECK_HEADER(sys/sysctl.h,AC_DEFINE([HAVE_SYS_SYSCTL],[1],[Define whether sys/sysctl.h is present]),)

AC_CHECK_HEADER(mach/mach.h,AC_DEFINE([HAVE_MACH_MACH],[1],[Define whether mach/mach.h is present]),)

if test "x${prefix}" = "xNONE"; then
  AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${ac_default_prefix}/share/bibledit", [Package data directory])
else
  AC_DEFINE_UNQUOTED(PACKAGE_DATA_DIR, "${prefix}/share/bibledit", [Package data directory])
fi

AC_DEFINE_UNQUOTED(PACKAGE_PREFIX_DIR, "${prefix}", [Package prefix directory])

# Check whether function 'stoi' is available
AC_MSG_CHECKING([whether stoi is available])
AC_COMPILE_IFELSE(
[AC_LANG_SOURCE([[
#include <string>
using namespace std;
int i = stoi ("string");
  ]])],
  [AC_DEFINE([HAVE_STOI], [1], [Define whether function 'stoi' is available])
   AC_MSG_RESULT([yes])],
  [AC_MSG_RESULT([no])]
)

# Check ICU Unicode library whether available.
PKG_CHECK_MODULES(ICU, icu-i18n >= 60.0, AC_DEFINE(HAVE_ICU, 1, Define whether ICU is available), AC_MSG_NOTICE(libicui18n development version >= 60.0 is needed for the Cloud.))
AC_SUBST(ICU_CFLAGS)
AC_SUBST(ICU_LIBS)

# Check UTF8 proc library whether available.
PKG_CHECK_MODULES(UTF8PROC, libutf8proc >= 2.3.0, AC_DEFINE(HAVE_UTF8PROC, 1, Define whether utf8proc is available), AC_MSG_NOTICE(libutf8proc development version >= 2.3.0 is beneficial to have.))
AC_SUBST(UTF8PROC_CFLAGS)
AC_SUBST(UTF8PROC_LIBS)

# Check the gumbo library  whether available.
PKG_CHECK_MODULES(GUMBO, gumbo >= 0.10.0,,AC_MSG_ERROR(The gumbo library development version >= 0.10.0 is needed.))
AC_SUBST(GUMBO_CFLAGS)
AC_SUBST(GUMBO_LIBS)

# Check the tidy library  whether available.
PKG_CHECK_MODULES(TIDY, tidy >= 5.4.0,,AC_MSG_ERROR(The tidy library development version >= 5.4.0 is needed.))
AC_SUBST(TIDY_CFLAGS)
AC_SUBST(TIDY_LIBS)

# Check the pugixml library whether available.
PKG_CHECK_MODULES(PUGIXML, pugixml >= 1.10, AC_DEFINE(HAVE_PUGIXML, 1, Define whether pugixml is available), AC_MSG_NOTICE(pugixml development version >= 1.10 is beneficial to have.))
AC_SUBST(PUGIXML_CFLAGS)
AC_SUBST(PUGIXML_LIBS)

AC_MSG_CHECKING([C++ compiler])
GPPVERSION=`${CXX} --version | grep clang`
if test "$GPPVERSION" = ""; then
AC_MSG_RESULT([GCC])
else
AC_MSG_RESULT([clang])
fi
AM_CONDITIONAL([COMPILERGCC],[test "$GPPVERSION" = ""])
AM_CONDITIONAL([COMPILERCLANG],[test "$GPPVERSION" != ""])


# Windows.
ENABLEWINDOWS=no
AC_ARG_ENABLE(windows,
[  --enable-windows Compile for Windows],
[
AC_DEFINE([WIN32], [1], [Define whether to compile on Windows])
AC_DEFINE([HAVE_WINDOWS], [1], [Define whether to compile on Windows])
ENABLEWINDOWS=yes
]
)

# Android.
ENABLEANDROID=no
AC_ARG_ENABLE(android,
[  --enable-android Compile for Android],
[
AC_DEFINE([HAVE_ANDROID], [1], [Define whether to compile for Android])
ENABLEANDROID=yes
]
)

# macOS.
ENABLEMAC=no
AC_ARG_ENABLE(mac,
[  --enable-mac Compile for Mac],
[
AC_DEFINE([HAVE_MAC], [1], [Define whether to compile for Mac])
ENABLEMAC=yes
]
)

# Linux.
ENABLELINUX=no
AC_ARG_ENABLE(linux,
[  --enable-linux Compile for Linux],
[
AC_DEFINE([HAVE_LINUX], [1], [Define whether to compile for Linux])
ENABLELINUX=yes
]
)

# iOS.
ENABLEIOS=no
AC_ARG_ENABLE(ios,
[  --enable-ios Compile for iOS],
[
AC_DEFINE([HAVE_IOS], [1], [Define whether to compile for iOS])
ENABLEIOS=yes
]
)

# Whether to enable compiler warnings.
ENABLEWARNINGS=no
AC_ARG_ENABLE(warnings,
[  --enable-warnings Enable compiler warnings],[ENABLEWARNINGS=yes])
AM_CONDITIONAL([COMPILERWARNINGS],[test $ENABLEWARNINGS = yes])


AC_CONFIG_FILES([Makefile])
AC_OUTPUT

echo "
Bibledit $VERSION configuration summary:

Installation prefix                                  : ${prefix}
Windows               (--enable-windows)             : ${ENABLEWINDOWS}
Android               (--enable-android)             : ${ENABLEANDROID}
macOS                 (--enable-mac)                 : ${ENABLEMAC}
Linux                 (--enable-linux)               : ${ENABLELINUX}
iOS                   (--enable-ios)                 : ${ENABLEIOS}
Enable GCC warnings   (--enable-warnings)            : ${ENABLEWARNINGS}
"
