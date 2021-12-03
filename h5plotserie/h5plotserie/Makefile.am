include $(top_srcdir)/qt.mk

if COND_WIN32
  MAYBE_WIN32_SRC = h5plotserie.rc
else
  MAYBE_SIGWATCH_SRC = qt-unix-signals/sigwatch.cpp
  MAYBE_SIGWATCH_MOC = qt-unix-signals/sigwatch.moc.cc
endif

bin_PROGRAMS = h5plotserie
h5plotserie_CPPFLAGS = -I$(top_srcdir) $(QT_CFLAGS) $(HDF5SERIE_CFLAGS) $(QWT_CFLAGS)
h5plotserie_LDADD    = $(QT_LIBS) $(HDF5SERIE_LIBS) $(QWT_LIBS) $(LIBDL)
h5plotserie_SOURCES = $(MAYBE_WIN32_SRC) $(MAYBE_SIGWATCH_SRC) main.cc \
  curves.cc \
  dataselection.cc \
  mainwindow.cc \
  abstractviewfilter.cc \
  plotarea.cc

noinst_HEADERS = curves.h \
  dataselection.h \
  mainwindow.h \
  abstractviewfilter.h \
  plotarea.h \
  plotdata.h \
  treewidgetitem.h

nodist_h5plotserie_SOURCES=$(QT_BUILT_SOURCES)
# C++ source file to build automatically (e.g. abc.moc.cc by abc.h)
QT_BUILT_SOURCES = dataselection.moc.cc
BUILT_SOURCES = $(QT_BUILT_SOURCES) $(MAYBE_SIGWATCH_MOC)

icondir = @datadir@/h5plotserie/icons
dist_icon_DATA = h5plotserie.svg

licdir = @datadir@/h5plotserie/qt-unix-signals
lic_DATA = qt-unix-signals/LICENCE

# plugins
pluginsiconenginesdir = @bindir@/iconengines
pluginsplatformsdir = @bindir@/platforms
pluginsstylesdir = @bindir@/styles
pluginsplatformthemesdir = @bindir@/platformthemes
pluginsiconengines_DATA = @QT_PLUGINS@/iconengines/*qsvgicon*
if COND_WIN32
pluginsplatforms_DATA = @QT_PLUGINS@/platforms/qwindows.dll
pluginsstyles_DATA = @QT_PLUGINS@/styles/qwindowsvistastyle.dll
else
pluginsplatforms_DATA = @QT_PLUGINS@/platforms/libqxcb.so
pluginsstyles_DATA = @QT_PLUGINSSTYLES_DATA@
pluginsplatformthemes_DATA = @QT_PLUGINSPLATFORMTHEMES_DATA@
endif

# freedesktop.org
freedesktoporgdir = @datadir@/mbsim-env/freedesktop.org
windowsdir = @datadir@/mbsim-env/windows
freedesktoporg_DATA = mbsim-env.de.h5plotserie.xml mbsim-env.de.h5plotserie.desktop mimeapps-h5plotserie.list h5plotserie.svg
windows_DATA = createLnk.vbs mbsim-env.de.h5plotserie.source.bat
bin_SCRIPTS = desktopIntegration.sh desktopIntegration.bat
