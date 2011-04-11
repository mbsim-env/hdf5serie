dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.57])
AC_INIT([h5plotserie],[0.0.1],[schneidm@users.berlios.de])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([h5plotserie/mainwindow.h])
AC_PROG_LIBTOOL # automatically adds lib search path to executable; helps linking on other platforms
AC_PROG_CXX
AC_PROG_CXXCPP
AC_LANG([C++])

dnl By autoscan
AC_C_CONST
AC_HEADER_STDBOOL

dnl Qt
PKG_CHECK_MODULES(QT, [QtCore >= 4.4 QtGui >= 4.4 QtXml >= 4.4])
dnl get moc, uic and rcc
test -z $MOC && { MOC=$(pkg-config.exe --variable moc_location QtCore) || MOC=moc; }
test -z $UIC && { UIC=$(pkg-config.exe --variable uic_location QtCore) || UIC=uic; }
test -z $RCC && { RCC=$(dirname $(which $MOC))/rcc || RCC=rcc; }
AC_SUBST([MOC])
AC_SUBST([UIC])
AC_SUBST([RCC])

LDFLAGS="$LDFLAGS -Wl,--no-undefined"

dnl HDF5Serie
PKG_CHECK_MODULES(HDF5SERIE, hdf5serie)

dnl qwt >= 5
AC_ARG_WITH(qwt_inc_prefix,
  [  --with-qwt-inc-prefix=PFX  Prefix where qwt headers are installed],
  [QWT_CFLAGS="-I$withval"],
  [QWT_CFLAGS="-I$prefix/include"])
AC_ARG_WITH(qwt_lib_prefix,
  [  --with-qwt-lib-prefix=PFX  Prefix where qwt library is installed],
  [QWT_LIBS="-L$withval -lqwt"],
  [QWT_LIBS="-L$prefix/lib -lqwt"])
OLD=$CPPFLAGS; CPPFLAGS="$CPPFLAGS $QWT_CFLAGS $QT_CFLAGS"
AC_CHECK_HEADERS([qwt_wheel.h], [],
  [AC_MSG_ERROR([Can not find qwt >= 5!])])
CPPFLAGS=$OLD
AC_SUBST([QWT_CFLAGS])
AC_SUBST([QWT_LIBS])

CPPFLAGS="$CPPFLAGS -Wall -Werror"

AC_CONFIG_FILES([Makefile h5plotserie/Makefile])

AC_OUTPUT
