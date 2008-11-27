SUBDIRS = . readwritetest

lib_LTLIBRARIES = libhdf5rereadmetadata.la
libhdf5rereadmetadata_la_SOURCES = h5filero.cc \
  groupro.cc

libhdf5rereadmetadata_la_CPPFLAGS = -I..

hdf5rereadmetadataincludedir = $(includedir)/hdf5serie
hdf5rereadmetadatainclude_HEADERS = h5filero.h \
  groupro.h \
  serie2dro.h \
  h5rereadmetadata.h
