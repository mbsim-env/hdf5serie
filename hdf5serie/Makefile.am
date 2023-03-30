SUBDIRS = hdf5serie
DIST_SUBDIRS = hdf5serie doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = hdf5serie.pc

licdir = @datadir@/hdf5serie
lic_DATA = COPYING NOTICE



# doxygen
.PHONY: doc doc_install doc_uninstall
doc:
	(cd doc && $(MAKE) $(AM_MAKEFLAGS) all)
doc_install:
	(cd doc && $(MAKE) $(AM_MAKEFLAGS) install)
doc_uninstall:
	(cd doc && $(MAKE) $(AM_MAKEFLAGS) uninstall)
