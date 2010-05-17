include $(top_srcdir)/qt.mk

bin_PROGRAMS = h5plotserie
h5plotserie_CPPFLAGS = $(QT_CFLAGS) $(HDF5SERIE_CFLAGS) $(QWT_CFLAGS)
h5plotserie_LDFLAGS  = 
h5plotserie_LDADD    = $(QT_LIBS) $(HDF5SERIE_LIBS) $(QWT_LIBS)
h5plotserie_SOURCES = main.cc\
  mainwindow.cc

nodist_h5plotserie_SOURCES=$(QT_BUILT_SOURCES)

# C++ source file to build automatically (e.g. abc.moc.cc by abc.h)
QT_BUILT_SOURCES = mainwindow.moc.cc

noinst_HEADERS = mainwindow.h\
  treewidgetitem.h

