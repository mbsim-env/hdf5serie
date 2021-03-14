dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.57])
AC_INIT([h5plotserie],[0.0.1],[markus.ms.schneider@googlemail.com])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_SRCDIR([h5plotserie/mainwindow.h])
LT_INIT([win32-dll])
AC_PROG_CXX
AC_PROG_CXXCPP
AC_LANG([C++])

# enable C++11
CXXFLAGS="$CXXFLAGS -std=c++17 -DQT_NO_KEYWORDS"

dnl By autoscan
AC_C_CONST
AC_HEADER_STDBOOL
AC_CHECK_FUNCS([setlocale])
AC_TYPE_SIZE_T

# windres
AC_ARG_WITH(windres,
  [  --with-windres=prog  The windres program (defaults to windres)],
  [WINDRES="$withval"],
  [WINDRES="windres"])
AC_SUBST([WINDRES])

dnl Qt
PKG_CHECK_MODULES(QT, [Qt5Core >= 5.6 Qt5Gui >= 5.6 Qt5Widgets >= 5.6 Qt5Xml >= 5.6])
dnl get moc and uic
test -z $MOC && { MOC=$(pkg-config --variable=host_bins Qt5Core)/moc || MOC=moc; }
test -z $UIC && { UIC=$(pkg-config --variable=host_bins Qt5Core)/uic || UIC=uic; }
AC_SUBST([MOC])
AC_SUBST([UIC])

AC_ARG_WITH(qmake, [  --with-qmake=PATH qmake program],
  [QMAKE="$withval"], [QMAKE="qmake"])
QT_PLUGINS=$($QMAKE -query QT_INSTALL_PLUGINS)
AC_SUBST([QT_PLUGINS])
AC_SUBST([QT_PLUGINSSTYLES_DATA], [$(ls -1 $QT_PLUGINS/styles/*.so 2> /dev/null | tr '\n' ' ')])
AC_SUBST([QT_PLUGINSPLATFORMTHEMES_DATA], [$(ls -1 $QT_PLUGINS/platformthemes/*.so 2> /dev/null | tr '\n' ' ')])

AM_CONDITIONAL([COND_WIN32], [test "_$host_os" = "_mingw32"])

AC_SUBST([HOST_OS],[$host_os])
if test "_$host_os" != "_mingw32"; then
  CPPFLAGS="$CPPFLAGS -fPIC"
fi

dnl shared libraries
if test "_$host_os" != "_mingw32"; then
  LIBDL="-ldl"
fi
AC_SUBST([LIBDL])

dnl HDF5Serie
PKG_CHECK_MODULES(HDF5SERIE, hdf5serie)

dnl qwt >= 6
AC_ARG_WITH(qwt_inc_prefix,
  [  --with-qwt-inc-prefix=PFX  Dir where qwt headers are installed],
  [QWT_CFLAGS="-I$withval"],
  [QWT_CFLAGS="-I$prefix/include"])
AC_ARG_WITH(qwt_lib_name,
  [  --with-qwt-lib-name=name  Qwt library name, used as -l<name>, needs a filename called lib<name>.so on Linux. (defaults to qwt)],
  [QWT_LIBNAME="$withval"],
  [QWT_LIBNAME="qwt"])
AC_ARG_WITH(qwt_lib_prefix,
  [  --with-qwt-lib-prefix=PFX  Dir where qwt library is installed],
  [QWT_LIBS="-L$withval -l$QWT_LIBNAME"],
  [QWT_LIBS="-L$prefix/lib -l$QWT_LIBNAME"])
OLDCPPFLAGS=$CPPFLAGS; CPPFLAGS="$CPPFLAGS $QWT_CFLAGS $QT_CFLAGS -std=c++17"
OLDLIBS=$LIBS; LIBS="$LIBS $QWT_LIBS $QT_LIBS"
AC_CHECK_HEADERS([qwt_system_clock.h], [], [AC_MSG_ERROR([Cannot find qwt_system_clock.h! Is qwt6 for qt5 installed? (Provide --with-qwt-inc-prefix=... if needed, see options)])])
AC_TRY_LINK(
  [#include <qwt_system_clock.h>],
  [new QwtSystemClock;],
  [],
  [AC_MSG_ERROR([Cannot link with qwt6! Is qwt6 for qt5 installed? (Provide --with-qwt-lib-prefix=... and/or --with-qwt-lib-name=... if needed, see options)])])
CPPFLAGS=$OLDCPPFLAGS
LIBS=$OLDLIBS
AC_SUBST([QWT_CFLAGS])
AC_SUBST([QWT_LIBS])

CPPFLAGS="$CPPFLAGS -Wall -Werror"

if test "_$host_os" = "_mingw32"; then
  LDFLAGS="-mwindows $LDFLAGS"
fi

LDFLAGS="$LDFLAGS $LDFLAGS_LIBTOOL -no-undefined -Wl,--no-undefined"
test "_$host_os" != "_mingw32" && LDFLAGS="$LDFLAGS -Wl,--disable-new-dtags,-rpath,\\\$\$ORIGIN/../lib"

AC_CONFIG_FILES([Makefile h5plotserie/Makefile])

hardcode_into_libs=no # do not add hardcoded libdirs to ltlibraries
hardcode_libdir_flag_spec_CXX= # do not add hardcodeed libdirs to ltbinaries
AC_OUTPUT
