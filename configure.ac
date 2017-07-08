#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([xwefax], [2.1], [nkcyham@yahoo.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

# Set CFLAGS if not set in environment
AC_MSG_CHECKING([whether configure should try to set CFLAGS])
if test "x${CFLAGS+set}" = xset; then
   enable_cflags_setting=no
else
   enable_cflags_setting=yes
fi
AC_MSG_RESULT($enable_cflags_setting)
test "x$enable_cflags_setting" = xyes && CFLAGS=""

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
pkg_modules="gtk+-2.0 >= 2.0.0"
PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)
AC_CHECK_LIB([m], [hypot])
AC_CHECK_LIB([asound], [snd_pcm_open])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([perseus-sdr], [perseus_init])

# Internationalization tools
IT_PROG_INTLTOOL([0.50.0])
GETTEXT_PACKAGE=xwefax
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Gettext package.])
AM_GLIB_GNU_GETTEXT
PROGRAMNAME_LOCALEDIR=[${datadir}/locale]
AC_SUBST(PROGRAMNAME_LOCALEDIR)

# Add the languages which your application supports here.
ALL_LINGUAS=""
AM_GLIB_GNU_GETTEXT

# Checks for header files.
AC_CHECK_HEADERS([libintl.h string.h sys/ioctl.h termios.h unistd.h])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([bzero memset strstr])

AC_CONFIG_FILES([Makefile
                 src/Makefile
				 po/Makefile.in])
AC_OUTPUT

