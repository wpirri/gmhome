AC_PREREQ(2.57)
AC_INIT([gmhome], [UPS-Monitor], [gnumonitor@gnumonitor.com.ar])
AC_CONFIG_SRCDIR([client/ups.cc])
AC_CONFIG_SRCDIR([server/statups.cc])
AM_INIT_AUTOMAKE
AC_PROG_LIBTOOL
#AC_PROG_RANLIB
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

# Checks for libraries.
#AC_CHECK_LIB (LIBRARY, FUNCTION)
# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([alarm dup2 memset strerror strstr])

#
# Activa la inclusion de informacion de debug en los binarios y libs
#AC_SUBST(gmhome_debug_global, "-ggdb")
# Ademas de la informacion de debug incluye mensajes de ejecucion
#AC_SUBST(gmhome_debug_global, "-ggdb -DDEBUG")

AC_OUTPUT( Makefile \
           client/Makefile \
           server/Makefile )
