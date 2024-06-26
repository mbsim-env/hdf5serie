include $(top_srcdir)/qt.mk

if COND_WIN32
  MAYBE_WIN32_h5plotserie_OBJ = h5plotserie.o
else
  MAYBE_SIGWATCH_SRC = qt-unix-signals/sigwatch.cpp
  MAYBE_SIGWATCH_MOC = qt-unix-signals/sigwatch.moc.cc
endif

# windows rc rules
h5plotserie.o: $(srcdir)/h5plotserie.rc $(srcdir)/h5plotserie.ico $(srcdir)/h5plotserie.manifest
	$(WINDRES) $< -o $@

bin_PROGRAMS = h5plotserie
h5plotserie_CPPFLAGS = -I$(top_srcdir) $(QT_CFLAGS) $(HDF5SERIE_CFLAGS) $(QWT_CFLAGS)
h5plotserie_LDADD    = $(QT_LIBS) $(HDF5SERIE_LIBS) $(QWT_LIBS) $(LIBDL) $(MAYBE_WIN32_h5plotserie_OBJ)
h5plotserie_SOURCES = $(MAYBE_SIGWATCH_SRC) main.cc \
  curves.cc \
  dataselection.cc \
  mainwindow.cc \
  abstractviewfilter.cc \
  plotarea.cc \
  dialogs.cc

noinst_HEADERS = curves.h \
  dataselection.h \
  mainwindow.h \
  abstractviewfilter.h \
  plotarea.h \
  plotdata.h \
  treewidgetitem.h \
  dialogs.h

nodist_h5plotserie_SOURCES=$(QT_BUILT_SOURCES)
# C++ source file to build automatically (e.g. abc.moc.cc by abc.h)
QT_BUILT_SOURCES = dataselection.moc.cc \
  abstractviewfilter.moc.cc
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
freedesktoporg_DATA = freedesktop.org/mbsim-env.h5plotserie.xml freedesktop.org/mbsim-env.h5plotserie.desktop freedesktop.org/mimeapps-h5plotserie.list h5plotserie.svg
bin_SCRIPTS = freedesktop.org/desktopIntegration.sh freedesktop.org/desktopIntegration.bat freedesktop.org/desktopIntegration.py
