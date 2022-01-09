bin_PROGRAMS = h5dumpserie h5lsserie h5lockserie

h5dumpserie_SOURCES = h5dumpserie.cc

h5dumpserie_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/hdf5serie $(FMATVEC_CFLAGS)
h5dumpserie_LDFLAGS = -L..
h5dumpserie_LDADD = ../libhdf5serie.la $(FMATVEC_LIBS) -l@BOOST_FILESYSTEM_LIB@ -l@BOOST_SYSTEM_LIB@


h5lsserie_SOURCES = h5lsserie.cc

h5lsserie_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/hdf5serie $(FMATVEC_CFLAGS)
h5lsserie_LDFLAGS = -L..
h5lsserie_LDADD = ../libhdf5serie.la $(FMATVEC_LIBS) -l@BOOST_FILESYSTEM_LIB@ -l@BOOST_SYSTEM_LIB@


h5lockserie_SOURCES = h5lockserie.cc

h5lockserie_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/hdf5serie $(FMATVEC_CFLAGS)
h5lockserie_LDADD = ../libhdf5serie.la $(FMATVEC_LIBS) -l@BOOST_FILESYSTEM_LIB@ -l@BOOST_PROGRAM_OPTIONS_LIB@ -l@BOOST_SYSTEM_LIB@


octavedir = @datadir@/hdf5serie/octave
dist_octave_DATA = h5dumpserie.m hdf5serieappenddataset.m
