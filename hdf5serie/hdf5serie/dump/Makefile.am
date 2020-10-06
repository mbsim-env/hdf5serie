bin_PROGRAMS = h5dumpserie h5lsserie h5flushserie

h5dumpserie_SOURCES = h5dumpserie.cc

h5dumpserie_CPPFLAGS = -I$(top_srcdir) $(FMATVEC_CFLAGS)
h5dumpserie_LDFLAGS = -L..
h5dumpserie_LDADD = ../libhdf5serie.la $(FMATVEC_LIBS) -l@BOOST_SYSTEM_LIB@


h5lsserie_SOURCES = h5lsserie.cc

h5lsserie_CPPFLAGS = -I$(top_srcdir) $(FMATVEC_CFLAGS)
h5lsserie_LDFLAGS = -L..
h5lsserie_LDADD = ../libhdf5serie.la $(FMATVEC_LIBS) -l@BOOST_FILESYSTEM_LIB@ -l@BOOST_SYSTEM_LIB@


h5flushserie_SOURCES = h5flushserie.cc

h5flushserie_CPPFLAGS = -I$(top_srcdir) $(FMATVEC_CFLAGS)
h5flushserie_LDADD = ../libhdf5serie.la $(FMATVEC_LIBS) -l@BOOST_FILESYSTEM_LIB@ -l@BOOST_SYSTEM_LIB@


octavedir = @datadir@/hdf5serie/octave
dist_octave_DATA = h5dumpserie.m hdf5serieappenddataset.m
