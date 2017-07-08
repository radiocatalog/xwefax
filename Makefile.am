## Process this file with automake to produce Makefile.in

SUBDIRS = src po

INTLTOOL_FILES = intltool-extract.in \
				 intltool-merge.in \
				 intltool-update.in

EXTRA_DIST = \
			 autogen.sh \
			 xwefax.glade \
			 xwefax.gladep \
			 xwefax.gladep \
			 doc \
			 xwefax \
			 $(INTLTOOL_FILES)

DISTCLEANFILES = \
				 intltool-extract* \
				 intltool-merge* \
				 intltool-update* \
				 po/.intltool-merge-cache*

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/pixmaps; then \
	  $(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/pixmaps; \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	  if test -f $$pixmap; then \
	  $(INSTALL_DATA) $$pixmap $(DESTDIR)$(pkgdatadir)/pixmaps; \
	  fi \
	  done \
	  fi
	
	if test -d $(HOME)/$(PACKAGE); then \
	  echo Working directory exists: $(HOME)/$(PACKAGE) - not copying; \
	  else \
	  echo Copying $(PACKAGE) working directory to $(HOME)/$(PACKAGE); \
	  cp -r $(srcdir)/$(PACKAGE) $(HOME); \
	  fi

dist-hook:
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	  if test -f $$pixmap; then \
	  cp -p $$pixmap $(distdir)/pixmaps; \
	  fi \
	  done \
	  fi

