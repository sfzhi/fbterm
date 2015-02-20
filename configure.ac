#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([FbTerm], [1.7])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/fbterm.cpp])
AC_CONFIG_HEADERS([config.h])

AC_SUBST([RELEASE_DATE], "October 2010")

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB
PKG_PROG_PKG_CONFIG

AC_ARG_ENABLE(gpm, 
              AC_HELP_STRING([--disable-gpm], [disable gpm mouse support [[default=auto]]]), 
              [GPM="$enableval"],
              [GPM=auto])
              
AC_ARG_ENABLE(vesa, 
              AC_HELP_STRING([--disable-vesa], [disable VESA video card support [[default=auto]]]), 
              [VESA="$enableval"],
              [VESA=auto])

AC_ARG_ENABLE(epoll, 
              AC_HELP_STRING([--disable-epoll], [do not use epoll system call [[default=auto]]]), 
              [EPOLL="$enableval"],
              [EPOLL=auto])

AC_ARG_ENABLE(signalfd,
              AC_HELP_STRING([--disable-signalfd], [do not use signalfd system call [[default=auto]]]),
              [SIGNALFD="$enableval"],
              [SIGNALFD=auto])

                                          
# Checks for libraries.
AC_CHECK_LIB([util], [forkpty])
PKG_CHECK_MODULES([FT2], [freetype2])
PKG_CHECK_MODULES([FC], [fontconfig])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h langinfo.h locale.h stddef.h stdlib.h string.h termios.h unistd.h \
				  sys/ioctl.h sys/socket.h sys/time.h linux/fb.h linux/kd.h linux/keyboard.h linux/input.h],
				  [], [AC_MSG_ERROR(required header file is missing!)])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_STRUCT_TM

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MMAP
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([memset memmove munmap nl_langinfo setenv setlocale socket strcasecmp strchr strstr strtol],
				[], [AC_MSG_ERROR(required lib function is missing!)])

GPM_WARN=
if test x"$GPM" != xno; then
	AC_CHECK_HEADERS([gpm.h])
	if test x"$GPM" = xauto; then
		GPM="$ac_cv_header_gpm_h"
		[[ "$GPM" = "no" ]] && GPM_WARN=yes 
	elif test x"$ac_cv_header_gpm_h" != xyes; then
		AC_MSG_ERROR(can't find gpm.h required by gpm mouse support!)
	fi
fi

VESA_WARN=
if test x"$VESA" != xno; then
	AC_CHECK_HEADERS([libx86.h])
	if test x"$VESA" = xauto; then
		VESA="$ac_cv_header_libx86_h"
		[[ "$VESA" = "no" ]] && VESA_WARN=yes
	elif test x"$ac_cv_header_libx86_h" != xyes; then
		AC_MSG_ERROR(can't find libx86.h required by VESA video card support!)
	fi
fi

if test x"$EPOLL" = xauto -a x"$cross_compiling" = xno; then
  AC_RUN_IFELSE(
    AC_LANG_PROGRAM([[#include <sys/epoll.h>]],
      [[if (epoll_create(10) >= 0) return 0; return 1;]]),
    [EPOLL=yes]
  )
fi

if test x"$SIGNALFD" = xauto -a x"$cross_compiling" = xno; then
  AC_RUN_IFELSE(
    AC_LANG_PROGRAM([[#include <sys/signalfd.h>]],
      [[sigset_t mask; if (signalfd(-1, &mask, 0) >= 0) return 0; return 1;]]),
    [SIGNALFD=yes]
  )
fi

AM_CONDITIONAL(VESA, test x"$VESA" = xyes)

x86_LIBS=
[[ "$VESA" = "yes" ]] && x86_LIBS="-lx86"
AC_SUBST([X86_LIBS], ["$x86_LIBS"])

AH_TEMPLATE([ENABLE_GPM])
AH_TEMPLATE([ENABLE_VESA])
AH_TEMPLATE([HAVE_EPOLL])
AH_TEMPLATE([HAVE_SIGNALFD])

[[ "$GPM" = "yes" ]] && AC_DEFINE([ENABLE_GPM])
[[ "$VESA" = "yes" ]] && AC_DEFINE([ENABLE_VESA])
[[ "$EPOLL" = "yes" ]] && AC_DEFINE([HAVE_EPOLL])
[[ "$SIGNALFD" = "yes" ]] && AC_DEFINE([HAVE_SIGNALFD])

AC_CONFIG_FILES([Makefile src/Makefile src/lib/Makefile im/Makefile terminfo/Makefile doc/Makefile doc/fbterm.1])
AC_CONFIG_LINKS([im/input_key.h:src/input_key.h im/immessage.h:src/immessage.h])
AC_OUTPUT

[[ "$GPM_WARN" = "yes" ]] && AC_MSG_WARN([gpm.h doesn't exist! gpm mouse support will be disabled!])
[[ "$VESA_WARN" = "yes" ]] && AC_MSG_WARN([libx86.h doesn't exist! VESA support will be disabled!]) || exit 0
