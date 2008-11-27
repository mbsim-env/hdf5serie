AC_PREREQ(2.61)
AC_INIT([hdf5serie],[1.0.0],[mafriedrich@users.berlios.de])
AC_CONFIG_SRCDIR([src/toh5type.cc])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE
AC_LANG([C++])

AC_PROG_LIBTOOL
AC_PROG_CXX

AC_C_CONST
# kill memcpy
AC_HEADER_STDC

dnl Check for hdf5 library
AC_ARG_WITH(hdf5-prefix,
  [  --with-hdf5-prefix=PFX  Prefix where hdf5 is installed],
  [CPPFLAGS="$CPPFLAGS -I$withval/include"
    LDFLAGS="$LDFLAGS -L$withval/lib"
    LIBS="$LIBS -lhdf5_cpp -lhdf5"], [])
AC_CHECK_HEADERS([H5Cpp.h], [],
  [AC_MSG_ERROR([Can not find hdf5 c++ header H5Cpp.h])])
AC_MSG_CHECKING([for hdf5 c++ library])
AC_TRY_LINK([#include <H5Cpp.h>],[H5::H5File file],[AC_MSG_RESULT([yes])],
  [AC_MSG_RESULT([no]); AC_MSG_ERROR([Can not link with hdf5 c++ library])])

AC_CONFIG_FILES([Makefile src/Makefile src/check/Makefile
  src/dump/Makefile hdf5serie.pc src/rereadmetadata/Makefile src/rereadmetadata/readwritetest/Makefile])

AC_OUTPUT
