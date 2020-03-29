AC_INIT([xml], [0.0.1], [andy_ball7@hotmail.com], [], [adreamingworld.com])
#AC_CANONICAL_HOST
#AC_CANONICAL_TARGET

AC_CONFIG_AUX_DIR(build-aux)
AM_INIT_AUTOMAKE([foreign])


AC_ARG_ENABLE([example],
[  --enable-example    Compile a simple program with the library],
[case "${enableval}" in
  yes) example=true ;;
  no)  example=false ;;
  *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],[example=false])
AM_CONDITIONAL([BUILD_TEST], [test x$example = xtrue])

AC_PROG_CC
AC_PROG_RANLIB
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([andxml.pc])

AC_OUTPUT

