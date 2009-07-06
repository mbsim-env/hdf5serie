include $(top_srcdir)/qt.mk

if COND_QWT
  MAYBE_QWT_CFLAGS = $(QWT_CFLAGS)
  MAYBE_QWT_LIBS = $(QWT_LIBS)
endif

bin_PROGRAMS = h5plotserie
h5plotserie_CPPFLAGS = $(QT_CFLAGS) $(HDF5SERIE_CFLAGS) $(MAYBE_QWT_CFLAGS)
h5plotserie_LDFLAGS  = 
h5plotserie_LDADD    = $(QT_LIBS) $(HDF5SERIE_LIBS) $(MAYBE_QWT_LIBS)
h5plotserie_SOURCES = $(MAYBE_WIN32_SRC)\
  main.cc\
  mainwindow.cc

nodist_h5plotserie_SOURCES=$(QT_BUILT_SOURCES)

# C++ source file to build automatically (e.g. abc.moc.cc by abc.h)
QT_BUILT_SOURCES = mainwindow.moc.cc

noinst_HEADERS = mainwindow.h\
  treewidgetitem.h

