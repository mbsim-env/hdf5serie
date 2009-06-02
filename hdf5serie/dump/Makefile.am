bin_PROGRAMS = h5dumpserie

h5dumpserie_SOURCES = h5dumpserie.cc

h5dumpserie_CPPFLAGS = -I..
h5dumpserie_LDFLAGS = -L..
h5dumpserie_LDADD = -lhdf5serie 



if COND_XSLT
  MAYBE_H5LSSERIE = h5lsserie
endif

bin_SCRIPTS = $(MAYBE_H5LSSERIE)

xsldir = @datadir@/hdf5serie/xsl
dist_xsl_DATA = h5lsserie.xsl



octavedir = @datadir@/hdf5serie/octave
dist_octave_DATA = h5dumpserie.m
