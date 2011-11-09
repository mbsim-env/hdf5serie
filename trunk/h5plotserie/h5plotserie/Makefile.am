include $(top_srcdir)/qt.mk

bin_PROGRAMS = h5plotserie
h5plotserie_CPPFLAGS = $(QT_CFLAGS) $(HDF5SERIE_CFLAGS) $(QWT_CFLAGS)
h5plotserie_LDFLAGS  = -Wl,-rpath -Wl,\$$ORIGIN/../lib
h5plotserie_LDADD    = $(QT_LIBS) $(HDF5SERIE_LIBS) $(QWT_LIBS)
h5plotserie_SOURCES = main.cc \
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

noinst_HEADERS = curves.h \
								 dataselection.h \
								 mainwindow.h \
								 plotarea.h \
								 plotdata.h \
								 treewidgetitem.h

