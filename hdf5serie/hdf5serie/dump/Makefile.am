bin_PROGRAMS = h5dumpserie h5lsserie h5lockserie

if COND_WIN32
  MAYBE_WIN32_h5dumpserie_OBJ = h5dumpserie.o
  MAYBE_WIN32_h5lockserie_OBJ = h5lockserie.o
  MAYBE_WIN32_h5lsserie_OBJ = h5lsserie.o
endif

# windows rc rules
h5dumpserie.o: $(srcdir)/h5dumpserie.rc $(srcdir)/h5dumpserie.manifest
	$(WINDRES) $< -o $@
h5lockserie.o: $(srcdir)/h5lockserie.rc $(srcdir)/h5lockserie.manifest
	$(WINDRES) $< -o $@
h5lsserie.o: $(srcdir)/h5lsserie.rc $(srcdir)/h5lsserie.manifest
	$(WINDRES) $< -o $@

h5dumpserie_SOURCES = h5dumpserie.cc

h5dumpserie_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/hdf5serie $(FMATVEC_CFLAGS)
h5dumpserie_LDFLAGS = -L..
h5dumpserie_LDADD = ../libhdf5serie.la $(FMATVEC_LIBS) -l@BOOST_FILESYSTEM_LIB@ -l@BOOST_SYSTEM_LIB@ $(MAYBE_WIN32_h5dumpserie_OBJ)


h5lsserie_SOURCES = h5lsserie.cc

h5lsserie_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/hdf5serie $(FMATVEC_CFLAGS)
h5lsserie_LDFLAGS = -L..
h5lsserie_LDADD = ../libhdf5serie.la $(FMATVEC_LIBS) -l@BOOST_FILESYSTEM_LIB@ -l@BOOST_SYSTEM_LIB@ $(MAYBE_WIN32_h5lsserie_OBJ)


h5lockserie_SOURCES = h5lockserie.cc

h5lockserie_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/hdf5serie $(FMATVEC_CFLAGS)
h5lockserie_LDADD = ../libhdf5serie.la $(FMATVEC_LIBS) -l@BOOST_FILESYSTEM_LIB@ -l@BOOST_PROGRAM_OPTIONS_LIB@ -l@BOOST_SYSTEM_LIB@ $(MAYBE_WIN32_h5lockserie_OBJ)


octavedir = @datadir@/hdf5serie/octave
dist_octave_DATA = h5dumpserie.m hdf5serieappenddataset.m
