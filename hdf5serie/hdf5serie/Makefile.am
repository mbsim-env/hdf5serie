SUBDIRS = . dump check

lib_LTLIBRARIES = libhdf5serie.la
libhdf5serie_la_SOURCES = toh5type.cc \
  simpleattribute.cc \
  simpledataset.cc \
  vectorserie.cc \
  fileserie.cc \
  matrixserie.cc

hdf5serieincludedir = $(includedir)/hdf5serie
libhdf5serie_la_CPPFLAGS = -I$(top_srcdir)

hdf5serieinclude_HEADERS = toh5type.h \
  simpleattribute.h \
  simpledataset.h\
  structserie.h \
  vectorserie.h \
  matrixserie.h \
  fileserie.h \
  knownpodtypes.def \
  knowntypes.def
