DIRFILE=$(THISDIR:%=$(DESTDIR)\..\%.dir)

.PHONY: destdir
destdir: $(DESTDIR)

all: destdir $(DATA_FILES) $(DIRFILE)

$(DESTDIR)\default: default.in
	copy $< $@

$(DESTDIR)\%: %
	copy $< $@

ifneq ($(DIRFILE),)
.PHONY: extrastuff

$(DIRFILE): extrastuff $(DATA_FILES)
	-del -e $@
	cd $(DESTDIR) & ..\..\xkbcomp.exe -lfhlpR -o $(relpath $@) *
endif
