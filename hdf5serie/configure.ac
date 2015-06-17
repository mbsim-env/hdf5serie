AC_PREREQ(2.59)
AC_INIT([hdf5serie],[2.0.0],[friedrich.at.gc@googlemail.com])
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
  WIN32=yes
fi
AM_CONDITIONAL([COND_WIN32], [test "_$WIN32" = "_yes"])

dnl Check for doxygen
AC_ARG_WITH([doxygenpath],
  [  --with-doxygenpath=PATH  The path to the 'doxygen' program.],
  [doxygenpath="$withval"],[doxygenpath=""])
AC_PATH_PROG([doxygen],[doxygen],[no],[$doxygenpath:$PATH])
if test $doxygen = "no" ; then
  AC_MSG_RESULT([can not generate API documentation. 'doxygen' is required.])
fi
#DOXYGENDEPTAGFILES=$(pkg-config deppackage --variable=DOXYGENTAGFILES)
#AC_SUBST([DOXYGENDEPTAGFILES])

PKG_CHECK_MODULES(FMATVEC, fmatvec)

dnl Check for hdf5 library
AC_ARG_WITH(hdf5_prefix,
  [  --with-hdf5-prefix=PFX  Prefix where hdf5 is installed],
  [HDF5CPPFLAGS="$HDF5CPPFLAGS -I$withval/include"
    HDF5LDFLAGS="$HDF5LDFLAGS -L$withval/lib"
    LDFLAGS="$LDFLAGS $HDF5LDFLAGS"
    LIBS="$LIBS -lhdf5"; hdf5_prefix=$withval],
  [HDF5CPPFLAGS="$HDF5CPPFLAGS -I$prefix/include"
    HDF5LDFLAGS="$HDF5LDFLAGS -L$prefix/lib"
    LDFLAGS="$LDFLAGS $HDF5LDFLAGS"
    LIBS="$LIBS -lhdf5"; hdf5_prefix=$prefix])
CPPFLAGS="$CPPFLAGS $HDF5CPPFLAGS"
if test "_$host_os" != "_mingw32" ; then
  LIBS="$LIBS -lrt -lm" # libs required by libhdf5
fi
LIBS="$LIBS -lz" # libs required by libhdf5
AC_SUBST([HDF5CPPFLAGS])
AC_SUBST([HDF5LDFLAGS])
AC_SUBST([hdf5_prefix])
AC_PATH_PROG([h5dump],[h5dump],[no],[$hdf5_prefix/bin:$PATH])
AC_CHECK_HEADERS([hdf5.h], [],
  [AC_MSG_ERROR([Can not find hdf5 c header hdf5.h])])
AC_MSG_CHECKING([for hdf5 c library])
AC_TRY_LINK([#include <hdf5.h>],[H5Fopen("dummy", H5F_ACC_RDWR, H5P_DEFAULT)],[AC_MSG_RESULT([yes])],
  [AC_MSG_RESULT([no]); AC_MSG_ERROR([Can not link with hdf5 c library])])
dnl check for swmr branch
AC_CHECK_LIB([hdf5], [H5Dflush], [AC_DEFINE([HDF5_SWMR], [1], [HDF5 SWMR branch detected])],
  [AC_MSG_NOTICE([To use SWMR features please use the HDF5 branch from svn: http://svn.hdfgroup.uiuc.edu/hdf5/branches/revise_chunks])])

# from now on only libtool is used for linking
LDFLAGS="$LDFLAGS -no-undefined -Wl,--no-undefined"
test "_$host_os" != "_mingw32" && LDFLAGS="$LDFLAGS -Wl,--disable-new-dtags,-rpath,\\\$\$ORIGIN/../lib"

# boost
AC_CHECK_HEADERS([boost/filesystem.hpp],[],[AC_MSG_ERROR([Can not find boost/filesystem.hpp])])

AC_ARG_WITH([boost-filesystem-lib],
  [  --with-boost-filesystem-lib=LIBNAME  The library name of boost filesystem excluding prefix/suffix (Default: boost_filesystem).],
  [BOOST_FILESYSTEM_LIB="$withval"], [BOOST_FILESYSTEM_LIB="boost_filesystem"])
AC_SUBST([BOOST_FILESYSTEM_LIB])
AC_ARG_WITH([boost-system-lib],
  [  --with-boost-system-lib=LIBNAME  The library name of boost system excluding prefix/suffix (Default: boost_system).],
  [BOOST_SYSTEM_LIB="$withval"], [BOOST_SYSTEM_LIB="boost_system"])
AC_SUBST([BOOST_SYSTEM_LIB])

AC_CONFIG_FILES([Makefile doc/Makefile hdf5serie/Makefile hdf5serie/check/Makefile
  hdf5serie/dump/Makefile hdf5serie.pc
  doc/Doxyfile hdf5serie/dump/h5dumpserie.m hdf5serie/dump/hdf5serieappenddataset.m])
AC_CONFIG_FILES([hdf5serie/check/testdump.sh],[chmod +x hdf5serie/check/testdump.sh])

AC_OUTPUT
