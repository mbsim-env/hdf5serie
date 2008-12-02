SUBDIRS = . dump check rereadmetadata

lib_LTLIBRARIES = libhdf5serie.la
libhdf5serie_la_SOURCES = toh5type.cc \
  simpleattribute.cc \
  simpledataset.cc \
  serie2d.cc

hdf5serieincludedir = $(includedir)/hdf5serie
hdf5serieinclude_HEADERS = toh5type.h \
  simpleattribute.h \
  simpledataset.h\
  serie2d.h \
  serie1d.h \
  knowntypes.def
