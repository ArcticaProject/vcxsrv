xpcdir = @xpconfigdir@/C/print/models/SPSPARC2/fonts

parentdir = ../../PSdefault/fonts

XPFONTS = \
        Courier-Bold.pmf \
        Courier-BoldOblique.pmf \
        Courier-Oblique.pmf \
        Courier.pmf \
        Helvetica-Bold.pmf \
        Helvetica-BoldOblique.pmf \
        Helvetica-Oblique.pmf \
        Helvetica.pmf \
        Symbol.pmf \
        Times-Bold.pmf \
        Times-BoldItalic.pmf \
        Times-Italic.pmf \
        Times-Roman.pmf

dest = $(DESTDIR)$(xpcdir)

remove-stuff:
	for x in $(XPFONTS) ; do			\
		rm -f $(dest)/$$x	;		\
	done

	rm -f $(dest)/fonts.dir

install-data-hook: remove-stuff
	mkdir -p $(dest) ;								\
	for x in $(XPFONTS) ; do							\
		ln -s $(parentdir)/$$x $(dest)/$$x	;				\
	done

	$(MKFONTSCALE) -b -s -l $(dest)

uninstall-hook: remove-stuff
