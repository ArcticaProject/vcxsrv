SUBDIRS = attributes ddx-config models

xpcdir = @xpconfigdir@/C/print


Xprinters.ghostscript: $(srcdir)/Xprinters
	sed < $(srcdir)/Xprinters "s/#Printer xp_pdf_spooldir_tmp_Xprintjobs/Printer xp_pdf_spooldir_tmp_Xprintjobs/" > Xprinters.ghostscript



dist_xpc_DATA = Xprinters
xpc_DATA = Xprinters.ghostscript

CLEANFILES = Xprinters.ghostscript
