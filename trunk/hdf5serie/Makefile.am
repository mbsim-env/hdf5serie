SUBDIRS = . dump check rereadmetadata

lib_LTLIBRARIES = libhdf5serie.la
libhdf5serie_la_SOURCES = toh5type.cc \
  simpleattribute.cc \
  simpledataset.cc \
  vectorserie.cc \
  matrixserie.cc

hdf5serieincludedir = $(includedir)/hdf5serie
hdf5serieinclude_HEADERS = toh5type.h \
  simpleattribute.h \
  simpledataset.h\
  structserie.h \
  vectorserie.h \
  matrixserie.h \
  knowntypes.def
