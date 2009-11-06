# Daniel Stone disowns all copyright on this file; no warranty is given as to its
# suitability or otherwise.

SUBDIRS = src sxpm cxpm

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = xpm.pc
EXTRA_DIST = \
	xpm.pc.in \
	xpm.PS.gz \
	CHANGES \
	COPYRIGHT \
	FAQ.html \
	FILES \
	README.AMIGA \
	README.html \
	README.MSW

EXTRA_DIST += ChangeLog
MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog
