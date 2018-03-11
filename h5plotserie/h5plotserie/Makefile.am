include $(top_srcdir)/qt.mk

if COND_WIN32
  MAYBE_WIN32_SRC = h5plotserie.rc
endif

bin_PROGRAMS = h5plotserie
h5plotserie_CPPFLAGS = -I$(top_srcdir) $(QT_CFLAGS) $(HDF5SERIE_CFLAGS) $(QWT_CFLAGS)
h5plotserie_LDADD    = $(QT_LIBS) $(HDF5SERIE_LIBS) $(QWT_LIBS)
h5plotserie_SOURCES = $(MAYBE_WIN32_SRC) main.cc \
  curves.cc \
  dataselection.cc \
  mainwindow.cc \
  plotarea.cc

nodist_h5plotserie_SOURCES=$(QT_BUILT_SOURCES)

# C++ source file to build automatically (e.g. abc.moc.cc by abc.h)
QT_BUILT_SOURCES = h5plotserie.qrc.cc curves.moc.cc \
  dataselection.moc.cc \
  mainwindow.moc.cc \
  plotarea.moc.cc
BUILT_SOURCES = $(QT_BUILT_SOURCES)

noinst_HEADERS = h5plotserie.qrc curves.h \
  dataselection.h \
  mainwindow.h \
  plotarea.h \
  plotdata.h \
  treewidgetitem.h

icondir = @datadir@/h5plotserie/icons
dist_icon_DATA = h5plotserie.svg

# plugins
pluginsiconenginesdir = @bindir@/iconengines
pluginsiconengines_DATA = @QT_PLUGINS@/iconengines/*qsvgicon*
pluginsplatformsdir = @bindir@/platforms
if COND_WIN32
pluginsplatforms_DATA = @QT_PLUGINS@/platforms/qwindows.dll
else
pluginsplatforms_DATA = @QT_PLUGINS@/platforms/libqxcb.so
endif

# some dependencies for h5plotserie.qrc.cc
h5plotserie.qrc.cc: h5plotserie.qrc h5plotserie.svg

# freedesktop.org
freedesktoporgdir = @datadir@/mbsim-env/freedesktop.org
freedesktoporg_DATA = mbsim-env.org.h5plotserie.xml mbsim-env.org.h5plotserie.desktop mimeapps-h5plotserie.list h5plotserie.svg
bin_SCRIPTS = desktopIntegration.sh
