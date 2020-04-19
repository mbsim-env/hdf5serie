include $(top_srcdir)/qt.mk

if COND_WIN32
  MAYBE_WIN32_SRC = h5plotserie.rc
endif

bin_PROGRAMS = h5plotserie
h5plotserie_CPPFLAGS = -I$(top_srcdir) $(QT_CFLAGS) $(HDF5SERIE_CFLAGS) $(QWT_CFLAGS)
h5plotserie_LDADD    = $(QT_LIBS) $(HDF5SERIE_LIBS) $(QWT_LIBS) $(LIBDL)
h5plotserie_SOURCES = $(MAYBE_WIN32_SRC) main.cc \
  curves.cc \
  dataselection.cc \
  mainwindow.cc \
  plotarea.cc

nodist_h5plotserie_SOURCES=$(QT_BUILT_SOURCES)

# C++ source file to build automatically (e.g. abc.moc.cc by abc.h)
QT_BUILT_SOURCES = curves.moc.cc \
  dataselection.moc.cc \
  mainwindow.moc.cc \
  plotarea.moc.cc
BUILT_SOURCES = $(QT_BUILT_SOURCES)

noinst_HEADERS = curves.h \
  dataselection.h \
  mainwindow.h \
  plotarea.h \
  plotdata.h \
  treewidgetitem.h

icondir = @datadir@/h5plotserie/icons
dist_icon_DATA = h5plotserie.svg

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
pluginsstyles_DATA = @QT_PLUGINS@/styles/adwaita.so \
                     @QT_PLUGINS@/styles/libbb10styleplugin.so \
                     @QT_PLUGINS@/styles/libqcleanlooksstyle.so \
                     @QT_PLUGINS@/styles/libqgtk2style.so \
                     @QT_PLUGINS@/styles/libqmotifstyle.so \
                     @QT_PLUGINS@/styles/libqplastiquestyle.so
pluginsplatformthemes_DATA = @QT_PLUGINS@/platformthemes/libqgtk2.so
endif

# freedesktop.org
freedesktoporgdir = @datadir@/mbsim-env/freedesktop.org
freedesktoporg_DATA = mbsim-env.de.h5plotserie.xml mbsim-env.de.h5plotserie.desktop mimeapps-h5plotserie.list h5plotserie.svg
bin_SCRIPTS = desktopIntegration.sh
