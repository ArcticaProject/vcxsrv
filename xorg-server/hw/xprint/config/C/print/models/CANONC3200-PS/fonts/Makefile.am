xpcdir = @xpconfigdir@/C/print/models/CANONC3200-PS/fonts

parentdir = ../../PSdefault/fonts

XPFONTS = \
        AvantGarde-Book.pmf \
        AvantGarde-BookOblique.pmf \
        AvantGarde-Demi.pmf \
        AvantGarde-DemiOblique.pmf \
        Courier-Bold.pmf \
        Courier-BoldOblique.pmf \
        Courier-Oblique.pmf \
        Courier.pmf \
        Helvetica-Bold.pmf \
        Helvetica-BoldOblique.pmf \
        Helvetica-Oblique.pmf \
        Helvetica.pmf \
        LubalinGraph-Book.pmf \
        LubalinGraph-BookOblique.pmf \
        LubalinGraph-Demi.pmf \
        LubalinGraph-DemiOblique.pmf \
        NewCentSchlbk-Bold.pmf \
        NewCentSchlbk-BoldItal.pmf \
        NewCentSchlbk-Ital.pmf \
        NewCentSchlbk-Roman.pmf \
        Souvenir-Demi.pmf \
        Souvenir-DemiItalic.pmf \
        Souvenir-Light.pmf \
        Souvenir-LightItalic.pmf \
        Symbol.pmf \
        Times-Bold.pmf \
        Times-BoldItalic.pmf \
        Times-Italic.pmf \
        Times-Roman.pmf \
        ZapfDingbats.pmf

dest = $(DESTDIR)$(xpcdir)

remove-stuff:
	for x in $(XPFONTS) ; do			\
		rm -f $(dest)/$$x	;		\
	done

	@rm -f $(dest)/fonts.dir

install-data-hook: remove-stuff
	mkdir -p $(dest) ;					\
	for x in $(XPFONTS) ; do				\
		ln -s $(parentdir)/$$x $(dest)/$$x	;	\
	done

	$(MKFONTSCALE) -b -s -l $(dest)

uninstall-hook: remove-stuff
