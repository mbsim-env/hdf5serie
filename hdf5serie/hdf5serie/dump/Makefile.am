bin_PROGRAMS = h5dumpserie h5lsserie

h5dumpserie_SOURCES = h5dumpserie.cc

h5dumpserie_CPPFLAGS = -I..
h5dumpserie_LDFLAGS = -L.. -Wl,-rpath -Wl,\$$ORIGIN/../lib
h5dumpserie_LDADD = ../libhdf5serie.la


h5lsserie_SOURCES = h5lsserie.cc

h5lsserie_CPPFLAGS = -I..
h5lsserie_LDFLAGS = -L.. -Wl,-rpath -Wl,\$$ORIGIN/../lib
h5lsserie_LDADD = ../libhdf5serie.la


octavedir = @datadir@/hdf5serie/octave
dist_octave_DATA = h5dumpserie.m hdf5serieappenddataset.m
