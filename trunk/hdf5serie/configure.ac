AC_PREREQ(2.59)
AC_INIT([hdf5serie],[1.0.1],[friedrich.at.gc@googlemail.com])
AC_CONFIG_SRCDIR([hdf5serie/toh5type.cc])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE
AC_LANG([C++])

LT_INIT([win32-dll])
AC_PROG_CXX

# By autoscan
AC_C_CONST
AC_C_INLINE
AC_HEADER_STDBOOL
AC_TYPE_PID_T

dnl force strict standard conformance
CPPFLAGS="$CPPFLAGS -Wall -Werror -pedantic-errors -Wno-long-long"
CFLAGS="$CFLAGS -std=iso9899:1990"
CXXFLAGS="$CXXFLAGS -std=c++98"

dnl Check for MinGW
AC_CANONICAL_HOST
if test "_$host_os" = "_mingw32" ; then
  AC_DEFINE([MINGW], [1], [Host System is MinGW/MSys])
  CPPFLAGS="$CPPFLAGS -Wno-uninitialized" # MinGW-g++-3.4 and -O2 => uninitialized value in stl header
fi

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

dnl Check for ANSI C Signal Handling
AC_MSG_CHECKING([for ANSI c signal SIGUSR2]); AC_COMPILE_IFELSE([
#include <signal.h>
int main() {
  signal(SIGUSR2, 0);
  return 0;
}
], [AC_MSG_RESULT([yes]); AC_DEFINE([HAVE_ANSICSIGNAL],[],[Use ANSI C Signal Handling])],
   [AC_MSG_RESULT([no])])

dnl Check for hdf5 library
AC_ARG_WITH(hdf5_prefix,
  [  --with-hdf5-prefix=PFX  Prefix where hdf5 is installed],
  [HDF5CPPFLAGS="$HDF5CPPFLAGS -I$withval/include -I$withval/include/cpp"
    HDF5LDFLAGS="-L$withval/lib"
    LDFLAGS="$LDFLAGS -L$withval/lib"
    LIBS="$LIBS -lhdf5_cpp -lhdf5"; hdf5_prefix=$withval],
  [HDF5CPPFLAGS="$HDF5CPPFLAGS -I$prefix/include -I$prefix/include/cpp"
    HDF5LDFLAGS="-L$prefix/lib"
    LDFLAGS="$LDFLAGS -L$prefix/lib"
    LIBS="$LIBS -lhdf5_cpp -lhdf5"; hdf5_prefix=$prefix])
CPPFLAGS="$CPPFLAGS $HDF5CPPFLAGS"
AC_SUBST([HDF5CPPFLAGS])
AC_SUBST([HDF5LDFLAGS])
AC_SUBST([hdf5_prefix])
AC_PATH_PROG([h5dump],[h5dump],[no],[$hdf5_prefix/bin:$PATH])
AC_CHECK_HEADERS([H5Cpp.h], [],
  [AC_MSG_ERROR([Can not find hdf5 c++ header H5Cpp.h])])
AC_MSG_CHECKING([for hdf5 c++ library])
LIBS="$LIBS -lz" # for deflate compression
AC_TRY_LINK([#include <H5Cpp.h>],[H5::H5File file],[AC_MSG_RESULT([yes])],
  [AC_MSG_RESULT([no]); AC_MSG_ERROR([Can not link with hdf5 c++ library])])

dnl Helper functions to convert path to windows
dirtowin() {
  echo $(cd $1 && pwd -W 2> /dev/null)
}
filetowin() {
  echo $(cd $(dirname $1) && pwd -W 2> /dev/null)/$(basename $1)
}
dnl Convert to Windows path
win_prefix=$(dirtowin $prefix)
AC_SUBST([win_prefix])
win_h5dump=$(filetowin $h5dump)
AC_SUBST([win_h5dump])

dnl Convert to native path
if pwd -W &> /dev/null; then
  native_prefix=$win_prefix
else
  native_prefix=$prefix
fi
AC_SUBST([native_prefix])

# from now on only libtool is used for linking
LDFLAGS="$LDFLAGS -no-undefined"

AC_CONFIG_FILES([Makefile hdf5serie/Makefile hdf5serie/check/Makefile
  hdf5serie/dump/Makefile hdf5serie.pc
  doc/Doxyfile hdf5serie/dump/h5dumpserie.m hdf5serie/dump/hdf5serieappenddataset.m])


AC_OUTPUT
