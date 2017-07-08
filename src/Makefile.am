## Process this file with automake to produce Makefile.in
AM_LDFLAGS = -Wl,--as-needed

AM_CFLAGS = --pedantic -Wall -std=gnu99 -O2 -D_FORTIFY_SOURCE=2 \
		 -fstack-protector -Wformat -Werror=format-security -fpie

AM_CPPFLAGS = \
	-DPROGRAMNAME_LOCALEDIR=\""$(PROGRAMNAME_LOCALEDIR)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = xwefax

xwefax_SOURCES = \
	callbacks.c callbacks.h \
	cat.c cat.h \
	detect.c detect.h \
	display.c display.h \
	dft.c dft.h \
	filters.c filters.h \
	interface.c interface.h \
	jpeg.c jpeg.h \
	main.c main.h \
	perseus.c perseus.h \
	shared.c shared.h \
	sound.c sound.h \
	stations.c stations.h \
	support.c support.h \
	utils.c utils.h \
	wefax.c wefax.h \
	common.h

xwefax_LDADD = @PACKAGE_LIBS@ $(INTLLIBS)

