#  Copyright 2005 Red Hat, Inc.
#
#  Permission to use, copy, modify, distribute, and sell this software
#  and its documentation for any purpose is hereby granted without
#  fee, provided that the above copyright notice appear in all copies
#  and that both that copyright notice and this permission notice
#  appear in supporting documentation, and that the name of Red Hat
#  not be used in advertising or publicity pertaining to distribution
#  of the software without specific, written prior permission.  Red
#  Hat makes no representations about the suitability of this software
#  for any purpose.  It is provided "as is" without express or implied
#  warranty.
#
#  RED HAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
#  INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
#  NO EVENT SHALL RED HAT BE LIABLE FOR ANY SPECIAL, INDIRECT OR
#  CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
#  OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
#  NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
#  CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

SGML_FILES = dmx.sgml scaled.sgml

if BUILD_LINUXDOC
TXT_FILES  = $(SGML_FILES:%.sgml=%.txt)
PS_FILES   = $(SGML_FILES:%.sgml=%.ps)
if BUILD_PDFDOC
PDF_FILES  = $(SGML_FILES:%.sgml=%.pdf)
endif
HTML_FILES = $(SGML_FILES:%.sgml=%.html)

SUFFIXES = .sgml .txt .html .ps .pdf

.sgml.txt:
	@rm -f $@
	$(MAKE_TEXT) $<

.sgml.ps:
	@rm -f $@
	$(MAKE_PS) $<

.ps.pdf:
	@rm -f $@
	$(MAKE_PDF) $<

.sgml.html:
	@rm -f $@
	$(MAKE_HTML) $<

noinst_DATA = $(TXT_FILES) $(PS_FILES) $(PDF_FILES) $(HTML_FILES)
CLEANFILES = $(TXT_FILES) $(PS_FILES) $(PDF_FILES) $(HTML_FILES)
endif

EXTRA_DIST = \
	$(SGML_FILES) \
	DMXSpec.txt \
	DMXSpec-v1.txt \
	dmx.txt \
	doxygen.conf \
	doxygen.css \
	doxygen.foot \
	doxygen.head \
	scaled.txt \
	html/annotated.html \
	html/ChkNotMaskEv_8c.html \
	html/ChkNotMaskEv_8h.html \
	html/ChkNotMaskEv_8h-source.html \
	html/classes.html \
	html/dmx_8h.html \
	html/dmx_8h-source.html \
	html/dmxarg_8c.html \
	html/dmxarg_8h.html \
	html/dmxarg_8h-source.html \
	html/dmxbackend_8c.html \
	html/dmxbackend_8h.html \
	html/dmxbackend_8h-source.html \
	html/dmxcb_8c.html \
	html/dmxcb_8h.html \
	html/dmxcb_8h-source.html \
	html/dmxclient_8h.html \
	html/dmxclient_8h-source.html \
	html/dmxcmap_8c.html \
	html/dmxcmap_8h.html \
	html/dmxcmap_8h-source.html \
	html/dmxcommon_8c.html \
	html/dmxcommon_8h.html \
	html/dmxcommon_8h-source.html \
	html/dmxcompat_8c.html \
	html/dmxcompat_8h.html \
	html/dmxcompat_8h-source.html \
	html/dmxconfig_8c.html \
	html/dmxconfig_8h.html \
	html/dmxconfig_8h-source.html \
	html/dmxconsole_8c.html \
	html/dmxconsole_8h.html \
	html/dmxconsole_8h-source.html \
	html/dmxcursor_8c.html \
	html/dmxcursor_8h.html \
	html/dmxcursor_8h-source.html \
	html/dmxdetach_8c.html \
	html/dmxdpms_8c.html \
	html/dmxdpms_8h.html \
	html/dmxdpms_8h-source.html \
	html/dmxdummy_8c.html \
	html/dmxdummy_8h.html \
	html/dmxdummy_8h-source.html \
	html/dmxeq_8c.html \
	html/dmxeq_8h.html \
	html/dmxeq_8h-source.html \
	html/dmxevents_8c.html \
	html/dmxevents_8h.html \
	html/dmxevents_8h-source.html \
	html/dmxext_8h.html \
	html/dmxext_8h-source.html \
	html/dmxextension_8c.html \
	html/dmxextension_8h.html \
	html/dmxextension_8h-source.html \
	html/dmxfont_8c.html \
	html/dmxfont_8h.html \
	html/dmxfont_8h-source.html \
	html/dmxgc_8c.html \
	html/dmxgc_8h.html \
	html/dmxgc_8h-source.html \
	html/dmxgcops_8c.html \
	html/dmxgcops_8h.html \
	html/dmxgcops_8h-source.html \
	html/dmx__glxvisuals_8h-source.html \
	html/dmxinit_8c.html \
	html/dmxinit_8h.html \
	html/dmxinit_8h-source.html \
	html/dmxinput_8c.html \
	html/dmxinput_8h.html \
	html/dmxinput_8h-source.html \
	html/dmxinputinit_8c.html \
	html/dmxinputinit_8h.html \
	html/dmxinputinit_8h-source.html \
	html/dmxlog_8c.html \
	html/dmxlog_8h.html \
	html/dmxlog_8h-source.html \
	html/dmxmap_8c.html \
	html/dmxmap_8h.html \
	html/dmxmap_8h-source.html \
	html/dmxmotion_8c.html \
	html/dmxmotion_8h.html \
	html/dmxmotion_8h-source.html \
	html/dmxparse_8c.html \
	html/dmxparse_8h.html \
	html/dmxparse_8h-source.html \
	html/dmxpict_8c.html \
	html/dmxpict_8h.html \
	html/dmxpict_8h-source.html \
	html/dmxpixmap_8c.html \
	html/dmxpixmap_8h.html \
	html/dmxpixmap_8h-source.html \
	html/dmxprint_8c.html \
	html/dmxprint_8h.html \
	html/dmxprint_8h-source.html \
	html/dmxprop_8c.html \
	html/dmxprop_8h.html \
	html/dmxprop_8h-source.html \
	html/dmxproto_8h.html \
	html/dmxproto_8h-source.html \
	html/dmxscrinit_8c.html \
	html/dmxscrinit_8h.html \
	html/dmxscrinit_8h-source.html \
	html/dmxshadow_8c.html \
	html/dmxshadow_8h.html \
	html/dmxshadow_8h-source.html \
	html/dmxsigio_8c.html \
	html/dmxsigio_8h.html \
	html/dmxsigio_8h-source.html \
	html/dmxstat_8c.html \
	html/dmxstat_8h.html \
	html/dmxstat_8h-source.html \
	html/dmxsync_8c.html \
	html/dmxsync_8h.html \
	html/dmxsync_8h-source.html \
	html/dmxvisual_8c.html \
	html/dmxvisual_8h.html \
	html/dmxvisual_8h-source.html \
	html/dmxwindow_8c.html \
	html/dmxwindow_8h.html \
	html/dmxwindow_8h-source.html \
	html/dmxxinput_8c.html \
	html/doxygen.css \
	html/doxygen.png \
	html/files.html \
	html/ftv2blank.png \
	html/ftv2doc.png \
	html/ftv2folderclosed.png \
	html/ftv2folderopen.png \
	html/ftv2lastnode.png \
	html/ftv2link.png \
	html/ftv2mlastnode.png \
	html/ftv2mnode.png \
	html/ftv2node.png \
	html/ftv2plastnode.png \
	html/ftv2pnode.png \
	html/ftv2vertline.png \
	html/functions.html \
	html/functions_vars.html \
	html/globals_defs.html \
	html/globals_enum.html \
	html/globals_eval.html \
	html/globals_func.html \
	html/globals.html \
	html/globals_type.html \
	html/globals_vars.html \
	html/index.html \
	html/lib_2dmx_2dmx_8c.html \
	html/lnx-keyboard_8c.html \
	html/lnx-keyboard_8h.html \
	html/lnx-keyboard_8h-source.html \
	html/lnx-ms_8c.html \
	html/lnx-ms_8h.html \
	html/lnx-ms_8h-source.html \
	html/lnx-ps2_8c.html \
	html/lnx-ps2_8h.html \
	html/lnx-ps2_8h-source.html \
	html/main.html \
	html/programs_2Xserver_2Xext_2dmx_8c.html \
	html/struct__dmxArg.html \
	html/struct__dmxColormapPriv.html \
	html/structDMXConfigCmdStruct.html \
	html/struct__DMXConfigComment.html \
	html/struct__DMXConfigDisplay.html \
	html/struct__DMXConfigEntry.html \
	html/struct__DMXConfigFullDim.html \
	html/structDMXConfigListStruct.html \
	html/struct__DMXConfigNumber.html \
	html/struct__DMXConfigOption.html \
	html/struct__DMXConfigPair.html \
	html/struct__DMXConfigParam.html \
	html/struct__DMXConfigPartDim.html \
	html/struct__DMXConfigString.html \
	html/struct__DMXConfigSub.html \
	html/struct__DMXConfigToken.html \
	html/struct__DMXConfigVirtual.html \
	html/struct__DMXConfigWall.html \
	html/struct__dmxCursorPriv.html \
	html/structDMXDesktopAttributes.html \
	html/structDMXDesktopAttributesRec.html \
	html/struct__DMXEventMap.html \
	html/struct__dmxFontPriv.html \
	html/struct__dmxGCPriv.html \
	html/structdmxGlxVisualPrivate.html \
	html/struct__dmxGlyphPriv.html \
	html/structDMXInputAttributes.html \
	html/structDMXInputAttributesRec.html \
	html/struct__DMXInputInfo.html \
	html/struct__DMXLocalInitInfo.html \
	html/struct__DMXLocalInputInfo.html \
	html/struct__dmxPictPriv.html \
	html/struct__dmxPixPriv.html \
	html/structDMXScreenAttributes.html \
	html/structDMXScreenAttributesRec.html \
	html/struct__DMXScreenInfo.html \
	html/struct__DMXStatAvg.html \
	html/struct__DMXStatInfo.html \
	html/structDMXWindowAttributes.html \
	html/structDMXWindowAttributesRec.html \
	html/struct__dmxWinPriv.html \
	html/struct__Event.html \
	html/struct__EventQueue.html \
	html/struct__myPrivate.html \
	html/structxDMXAddInputReply.html \
	html/structxDMXAddInputReq.html \
	html/structxDMXAddScreenReply.html \
	html/structxDMXAddScreenReq.html \
	html/structxDMXChangeDesktopAttributesReply.html \
	html/structxDMXChangeDesktopAttributesReq.html \
	html/structxDMXChangeScreensAttributesReply.html \
	html/structxDMXChangeScreensAttributesReq.html \
	html/structxDMXForceWindowCreationReply.html \
	html/structxDMXForceWindowCreationReq.html \
	html/structxDMXGetDesktopAttributesReply.html \
	html/structxDMXGetDesktopAttributesReq.html \
	html/structxDMXGetInputAttributesReply.html \
	html/structxDMXGetInputAttributesReq.html \
	html/structxDMXGetInputCountReply.html \
	html/structxDMXGetInputCountReq.html \
	html/structxDMXGetScreenAttributesReply.html \
	html/structxDMXGetScreenAttributesReq.html \
	html/structxDMXGetScreenCountReply.html \
	html/structxDMXGetScreenCountReq.html \
	html/structxDMXGetWindowAttributesReply.html \
	html/structxDMXGetWindowAttributesReq.html \
	html/structxDMXQueryVersionReply.html \
	html/structxDMXQueryVersionReq.html \
	html/structxDMXRemoveInputReply.html \
	html/structxDMXRemoveInputReq.html \
	html/structxDMXRemoveScreenReply.html \
	html/structxDMXRemoveScreenReq.html \
	html/structxDMXSyncReply.html \
	html/structxDMXSyncReq.html \
	html/tree.html \
	html/usb-common_8c.html \
	html/usb-common_8h.html \
	html/usb-common_8h-source.html \
	html/usb-keyboard_8c.html \
	html/usb-keyboard_8h.html \
	html/usb-keyboard_8h-source.html \
	html/usb-mouse_8c.html \
	html/usb-mouse_8h.html \
	html/usb-mouse_8h-source.html \
	html/usb-other_8c.html \
	html/usb-other_8h.html \
	html/usb-other_8h-source.html \
	html/usb-private_8h.html \
	html/usb-private_8h-source.html
