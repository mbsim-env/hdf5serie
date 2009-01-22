SUBDIRS = src

DIST_SUBDIRS = src doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = hdf5serie.pc



# doxygen
.PHONY: doc doc_install doc_uninstall
doc:
	cd doc; \
	rm -rf html; \
	$(doxygen)
doc_install:
	cd doc; \
	$(MKDIR_P) $(datadir)/doc/$(PACKAGE)-$(VERSION); \
	cp -r html $(datadir)/doc/$(PACKAGE)-$(VERSION); \
	cp $(PACKAGE)-$(VERSION).tag $(datadir)/doc/$(PACKAGE)-$(VERSION)
doc_uninstall:
	rm -rf $(datadir)/doc/$(PACKAGE)-$(VERSION)/html
	rm -rf $(datadir)/doc/$(PACKAGE)-$(VERSION)/$(PACKAGE)-$(VERSION).tag
