libmandir = $(LIB_MAN_DIR)

LIB_MAN_DIR_SUFFIX = $(LIB_MAN_DIR:@mandir@/man%=%)

libman_PRE = \
	DBE.man \
	DPMSCapable.man \
	DPMSDisable.man \
	DPMSEnable.man \
	DPMSForceLevel.man \
	DPMSGetTimeouts.man \
	DPMSGetVersion.man \
	DPMSInfo.man \
	DPMSQueryExtension.man \
	DPMSSetTimeouts.man \
	XShape.man \
	XShm.man \
	XcupGetReservedColormapEntries.man \
	XcupQueryVersion.man \
	XcupStoreColors.man \
	XdbeAllocateBackBufferName.man \
	XdbeBeginIdiom.man \
	XdbeDeallocateBackBufferName.man \
	XdbeEndIdiom.man \
	XdbeFreeVisualInfo.man \
	XdbeGetBackBufferAttributes.man \
	XdbeGetVisualInfo.man \
	XdbeQueryExtension.man \
	XdbeSwapBuffers.man \
	Xevi.man \
	Xmbuf.man

XShape_man_aliases = XShapeQueryExtension XShapeQueryVersion XShapeCombineRegion XShapeCombineRectangles XShapeCombineMask XShapeCombineShape XShapeOffsetShape XShapeQueryExtents XShapeSelectInput XShapeInputSelected XShapeGetRectangles

XShm_man_aliases = XShmQueryExtension XShmQueryVersion XShmPixmapFormat XShmAttach XShmDetach XShmCreateImage XShmPutImage XShmGetImage XShmCreatePixmap XShmGetEventBase

Xmbuf_man_aliases = XmbufQueryExtension XmbufGetVersion XmbufCreateBuffers XmbufDestroyBuffers XmbufDisplayBuffers XmbufGetWindowAttributes XmbufChangeWindowAttributes XmbufGetBufferAttributes XmbufChangeBufferAttributes XmbufGetScreenInfo XmbufCreateStereoWindow

Xevi_man_aliases = XeviQueryVersion XeviGetVisualInfo XeviQueryExtension

all_aliases = $(XShape_man_aliases) $(XShm_man_aliases) $(Xmbuf_man_aliases) $(Xevi_man_aliases)

libman_DATA = $(libman_PRE:man=@LIB_MAN_SUFFIX@) \
	      $(all_aliases:=.@LIB_MAN_SUFFIX@)

EXTRA_DIST  = $(libman_PRE)

CLEANFILES  = $(libman_DATA)

# Generate man page shadow files (Replaces InstallManPageAliases from Imake)

XShape_shadowmen = $(XShape_man_aliases:=.@LIB_MAN_SUFFIX@)

$(XShape_shadowmen):
	$(AM_V_GEN)echo .so man$(LIB_MAN_DIR_SUFFIX)/XShape.$(LIB_MAN_SUFFIX) > $@

XShm_shadowmen = $(XShm_man_aliases:=.@LIB_MAN_SUFFIX@)

$(XShm_shadowmen):
	$(AM_V_GEN)echo .so man$(LIB_MAN_DIR_SUFFIX)/XShm.$(LIB_MAN_SUFFIX) > $@

Xmbuf_shadowmen = $(Xmbuf_man_aliases:=.@LIB_MAN_SUFFIX@)

$(Xmbuf_shadowmen):
	$(AM_V_GEN)echo .so man$(LIB_MAN_DIR_SUFFIX)/Xmbuf.$(LIB_MAN_SUFFIX) > $@

Xevi_shadowmen = $(Xevi_man_aliases:=.@LIB_MAN_SUFFIX@)

$(Xevi_shadowmen):
	$(AM_V_GEN)echo .so man$(LIB_MAN_DIR_SUFFIX)/Xevi.$(LIB_MAN_SUFFIX) > $@

# Rules for generating files using the C pre-processor
# (Replaces CppFileTarget from Imake)

SED = sed

SUFFIXES = .$(LIB_MAN_SUFFIX) .man

# Translate XCOMM into pound sign with sed, rather than passing -DXCOMM=XCOMM
# to cpp, because that trick does not work on all ANSI C preprocessors.
# Delete line numbers from the cpp output (-P is not portable, I guess).
# Allow XCOMM to be preceded by whitespace and provide a means of generating
# output lines with trailing backslashes.
# Allow XHASH to always be substituted, even in cases where XCOMM isn't.

CPP_SED_MAGIC = $(SED) -e '/^\#  *[0-9][0-9]*  *.*$$/d' \
                       -e '/^\#line  *[0-9][0-9]*  *.*$$/d' \
                       -e '/^[         ]*XCOMM$$/s/XCOMM/\#/' \
                       -e '/^[         ]*XCOMM[^a-zA-Z0-9_]/s/XCOMM/\#/' \
                       -e '/^[         ]*XHASH/s/XHASH/\#/' \
                       -e '/\@\@$$/s/\@\@$$/\\/'

# Strings to replace in man pages
XORGRELSTRING = @PACKAGE_STRING@
  XORGMANNAME = X Version 11
  XSERVERNAME = Xorg

MANDEFS = \
        -D__vendorversion__="\"$(XORGRELSTRING)\" \"$(XORGMANNAME)\"" \
        -D__xorgversion__="\"$(XORGRELSTRING)\" \"$(XORGMANNAME)\"" \
	-D__appmansuffix__=$(APP_MAN_SUFFIX) \
	-D__filemansuffix__=$(FILE_MAN_SUFFIX) \
	-D__libmansuffix__=$(LIB_MAN_SUFFIX) \
	-D__miscmansuffix__=$(MISC_MAN_SUFFIX) \
	-D__drivermansuffix__=$(DRIVER_MAN_SUFFIX) \
	-D__adminmansuffix__=$(ADMIN_MAN_SUFFIX) \
	-D__projectroot__=$(prefix)

.man.$(LIB_MAN_SUFFIX):
	$(AM_V_GEN)$(RAWCPP) $(RAWCPPFLAGS) $(MANDEFS) $(EXTRAMANDEFS) < $< | $(CPP_SED_MAGIC) > $@

