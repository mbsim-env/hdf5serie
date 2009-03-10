AC_PREREQ(2.59)
AC_INIT([hdf5serie],[0.1.0],[mafriedrich@users.berlios.de])
AC_CONFIG_SRCDIR([hdf5serie/toh5type.cc])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE
AC_LANG([C++])

AC_PROG_LIBTOOL
AC_PROG_CXX

AC_C_CONST
AC_C_INLINE
AC_HEADER_STDBOOL

dnl Check for doxygen
AC_ARG_WITH([doxygenpath],
  [  --with-doxygenpath=PATH  The path to the 'doxygen' program.],
  [doxygenpath="$withval"],[doxygenpath=""])
AC_PATH_PROG([doxygen],[doxygen],[no],[$doxygenpath:$PATH])
if test $doxygen = "no" ; then
  AC_MSG_RESULT([can't generate API documentation. 'doxygen' is required.])
fi
#DOXYGENDEPTAGFILES=$(pkg-config deppackage --variable=DOXYGENTAGFILES)
#AC_SUBST([DOXYGENDEPTAGFILES])

dnl Check for hdf5 library
AC_ARG_WITH(hdf5_prefix,
  [  --with-hdf5-prefix=PFX  Prefix where hdf5 is installed],
  [CPPFLAGS="$CPPFLAGS -I$withval/include"
    LDFLAGS="$LDFLAGS -L$withval/lib"
    LIBS="$LIBS -lhdf5_cpp -lhdf5"; hdf5_prefix=$withval],
  [CPPFLAGS="$CPPFLAGS -I$prefix/include"
    LDFLAGS="$LDFLAGS -L$prefix/lib"
    LIBS="$LIBS -lhdf5_cpp -lhdf5"; hdf5_prefix=$prefix])
AC_PATH_PROG([h5dump],[h5dump],[no],[$hdf5_prefix/bin:$PATH])
AC_CHECK_HEADERS([H5Cpp.h], [],
  [AC_MSG_ERROR([Can not find hdf5 c++ header H5Cpp.h])])
AC_MSG_CHECKING([for hdf5 c++ library])
AC_TRY_LINK([#include <H5Cpp.h>],[H5::H5File file],[AC_MSG_RESULT([yes])],
  [AC_MSG_RESULT([no]); AC_MSG_ERROR([Can not link with hdf5 c++ library])])

AC_CONFIG_FILES([Makefile hdf5serie/Makefile hdf5serie/check/Makefile
  hdf5serie/dump/Makefile hdf5serie.pc hdf5serie/rereadmetadata/Makefile
  hdf5serie/rereadmetadata/readwritetest/Makefile
  doc/Doxyfile hdf5serie/dump/h5dumpserie.m])
AC_CONFIG_FILES([hdf5serie/dump/h5lsserie],[chmod +x hdf5serie/dump/h5lsserie])


AC_OUTPUT
