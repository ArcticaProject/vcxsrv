xpcdir = @xpconfigdir@/C/print/models/PSdefault/fonts

dist_xpc_DATA =				\
	AvantGarde-BookOblique.pmf	\
	AvantGarde-Book.pmf		\
	AvantGarde-DemiOblique.pmf	\
	AvantGarde-Demi.pmf		\
	Courier-BoldOblique.pmf		\
	Courier-Bold.pmf		\
	Courier-Oblique.pmf		\
	Courier.pmf			\
	Helvetica-BoldOblique.pmf	\
	Helvetica-Bold.pmf		\
	Helvetica-Oblique.pmf		\
	Helvetica.pmf			\
	LubalinGraph-BookOblique.pmf	\
	LubalinGraph-Book.pmf		\
	LubalinGraph-DemiOblique.pmf	\
	LubalinGraph-Demi.pmf		\
	NewCentSchlbk-Bold.pmf		\
	NewCentSchlbk-BoldItal.pmf 	\
	NewCentSchlbk-Ital.pmf		\
	NewCentSchlbk-Roman.pmf		\
	Souvenir-DemiItalic.pmf		\
	Souvenir-Demi.pmf		\
	Souvenir-LightItalic.pmf	\
	Souvenir-Light.pmf		\
	Symbol.pmf			\
	Times-BoldItalic.pmf		\
	Times-Bold.pmf			\
	Times-Italic.pmf		\
	Times-Roman.pmf			\
	ZapfDingbats.pmf


dest = $(DESTDIR)$(xpcdir)

remove-stuff:
	@rm -f $(dest)/fonts.dir

install-data-hook: remove-stuff
	$(MKFONTSCALE) -b -s -l $(dest)

uninstall-hook: remove-stuff
