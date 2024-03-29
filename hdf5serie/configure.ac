AC_PREREQ(2.59)
AC_INIT([hdf5serie],[2.0.0],[friedrich.at.gc@googlemail.com])
AC_CONFIG_SRCDIR([hdf5serie/toh5type.cc])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE
AC_LANG([C++])

# enable C++11
CXXFLAGS="$CXXFLAGS -std=c++17"

LT_INIT([win32-dll])
AC_PROG_CXX

# By autoscan
AC_C_CONST
AC_C_INLINE
AC_HEADER_STDBOOL
AC_TYPE_PID_T
AC_FUNC_ERROR_AT_LINE
# AC_FUNC_MALLOC cannot be added due to Windows build
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T

# windres
AC_ARG_WITH(windres,
  [  --with-windres=prog  The windres program (defaults to windres)],
  [WINDRES="$withval"],
  [WINDRES="windres"])
AC_SUBST([WINDRES])

dnl force strict standard conformance
CPPFLAGS="$CPPFLAGS -Wall -Werror -pedantic-errors -Wno-long-long"
CFLAGS="$CFLAGS -std=iso9899:1990"

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
  [HDF5CPPFLAGS="-I$withval/include"
   HDF5LDFLAGS="-L$withval/lib"
   hdf5_lib="$withval/lib"],
  [HDF5CPPFLAGS="-I$prefix/include"
   HDF5LDFLAGS="-L$prefix/lib"
   hdf5_lib="$prefix/lib"])
AC_ARG_WITH(hdf5_inc,
  [  --with-hdf5-inc=PFX  Include dir of hdf5],
  [HDF5CPPFLAGS="-I$withval"],
  [])
AC_ARG_WITH(hdf5_lib,
  [  --with-hdf5-lib=PFX  Library dir of hdf5],
  [HDF5LDFLAGS="-L$withval/lib"
   hdf5_lib="$withval"],
  [])
LDFLAGS="$LDFLAGS $HDF5LDFLAGS"
if [ ls $hdf5_lib/*hdf5-shared.* &> /dev/null ]; then # try the cmake generated shared name first
  LIBS="$LIBS -lhdf5-shared"
else # if not found use the "standard" name
  LIBS="$LIBS -lhdf5"
fi
CPPFLAGS="$CPPFLAGS $HDF5CPPFLAGS"
if test "_$host_os" != "_mingw32" ; then
  LIBS="$LIBS -lrt -lm" # libs required by libhdf5
fi
LIBS="$LIBS -lz" # libs required by libhdf5
AC_SUBST([HDF5CPPFLAGS])
AC_SUBST([HDF5LDFLAGS])
AC_CHECK_HEADERS([hdf5.h], [],
  [AC_MSG_ERROR([Can not find hdf5 c header hdf5.h])])

AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <hdf5.h>]],
[[
  #if !H5_VERSION_GE(1, 10, 0)
    #error "Version >= 1.10 needed"
  #endif
]])],
[AC_MSG_RESULT([OK])],
[AC_MSG_ERROR([Need at least HDF5 version >= 1.10])])

AC_MSG_CHECKING([for hdf5 c library])
AC_TRY_LINK([#include <hdf5.h>],[H5Fopen("dummy", H5F_ACC_RDWR, H5P_DEFAULT)],[AC_MSG_RESULT([yes])],
  [AC_MSG_RESULT([no]); AC_MSG_ERROR([Can not link with hdf5 c library])])

# from now on only libtool is used for linking
LDFLAGS="$LDFLAGS $LDFLAGS_LIBTOOL -no-undefined -Wl,--no-undefined"
test "_$host_os" != "_mingw32" && LDFLAGS="$LDFLAGS -Wl,--disable-new-dtags,-rpath,\\\$\$ORIGIN/../lib"

dnl Include path for Boost
AC_ARG_WITH([boost-inc],
  [  --with-boost-inc=PATH  The path Boost include directory.],
  [CPPFLAGS="$CPPFLAGS -I$withval"; DEFINES="$DEFINES -I$withval"])

# boost
AC_CHECK_HEADERS([boost/filesystem.hpp],[],[AC_MSG_ERROR([Can not find boost/filesystem.hpp])])

AC_ARG_WITH([boost-program-options-lib],
  [  --with-boost-program-options-lib=LIBNAME  The library name of boost program options excluding prefix/suffix (Default: boost_program_options).],
  [BOOST_PROGRAM_OPTIONS_LIB="$withval"], [BOOST_PROGRAM_OPTIONS_LIB="boost_program_options"])
AC_SUBST([BOOST_PROGRAM_OPTIONS_LIB])
AC_ARG_WITH([boost-thread-lib],
  [  --with-boost-thread-lib=LIBNAME  The library name of boost thread excluding prefix/suffix (Default: boost_thread).],
  [BOOST_THREAD_LIB="$withval"], [BOOST_THREAD_LIB="boost_thread"])
AC_SUBST([BOOST_THREAD_LIB])
AC_ARG_WITH([boost-chrono-lib],
  [  --with-boost-chrono-lib=LIBNAME  The library name of boost chrono excluding prefix/suffix (Default: boost_chrono).],
  [BOOST_CHRONO_LIB="$withval"], [BOOST_CHRONO_LIB="boost_chrono"])
AC_SUBST([BOOST_CHRONO_LIB])
AC_ARG_WITH([boost-filesystem-lib],
  [  --with-boost-filesystem-lib=LIBNAME  The library name of boost filesystem excluding prefix/suffix (Default: boost_filesystem).],
  [BOOST_FILESYSTEM_LIB="$withval"], [BOOST_FILESYSTEM_LIB="boost_filesystem"])
AC_SUBST([BOOST_FILESYSTEM_LIB])
AC_ARG_WITH([boost-system-lib],
  [  --with-boost-system-lib=LIBNAME  The library name of boost system excluding prefix/suffix (Default: boost_system).],
  [BOOST_SYSTEM_LIB="$withval"], [BOOST_SYSTEM_LIB="boost_system"])
AC_SUBST([BOOST_SYSTEM_LIB])

if test "$cross_compiling" == "yes"; then
  AC_SUBST([XC_EXEC_PREFIX],[wine])
fi

AC_CONFIG_FILES([Makefile doc/Makefile hdf5serie/Makefile hdf5serie/check/Makefile
  hdf5serie/dump/Makefile hdf5serie.pc
  doc/doxyfile hdf5serie/dump/h5dumpserie.m hdf5serie/dump/hdf5serieappenddataset.m])
AC_CONFIG_FILES([hdf5serie/check/testlib.sh],[chmod +x hdf5serie/check/testlib.sh])
AC_CONFIG_FILES([hdf5serie/check/testdump.sh],[chmod +x hdf5serie/check/testdump.sh])
AC_CONFIG_FILES([hdf5serie/check/valgrindtestdump.sh],[chmod +x hdf5serie/check/valgrindtestdump.sh])
AC_CONFIG_FILES([hdf5serie/check/valgrindtestlib.sh],[chmod +x hdf5serie/check/valgrindtestlib.sh])

hardcode_into_libs=no # do not add hardcoded libdirs to ltlibraries
hardcode_libdir_flag_spec_CXX= # do not add hardcodeed libdirs to ltbinaries
AC_OUTPUT
