DIRFILE=$(THISDIR:%=$(DESTDIR)\..\%.dir)
all: $(DESTDIR) $(DATA_FILES) $(DIRFILE)

$(DESTDIR)\default: default
	copy $< $@

$(DESTDIR)\%: %
	copy $< $@

ifneq ($(DIRFILE),)
.PHONY: extrastuff

$(DIRFILE): extrastuff $(DATA_FILES)
	-del -e $@
	cd $(DESTDIR) & ..\..\xkbcomp.exe -lfhlpR -o $(relpath $@) *
endif
