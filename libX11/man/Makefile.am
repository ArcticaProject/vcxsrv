# Have to fake out automake by calling our man pages "DATA" since "MANS" won't
# accept .3x as a valid man page suffix

SUBDIRS = xkb

libmandir = $(LIB_MAN_DIR)

LIB_MAN_DIR_SUFFIX = $(LIB_MAN_DIR:@mandir@/man%=%)

libman_PRE = \
	AllPlanes.man \
	BlackPixelOfScreen.man \
	DisplayOfCCC.man \
	ImageByteOrder.man \
	IsCursorKey.man \
	XAllocClassHint.man \
	XAllocIconSize.man \
	XAllocStandardColormap.man \
	XAllocSizeHints.man \
	XAllocWMHints.man \
	XAddHost.man \
	XAllocColor.man \
	XAllowEvents.man \
	XAnyEvent.man \
	XButtonEvent.man \
	XChangeKeyboardControl.man \
	XChangeKeyboardMapping.man \
	XClientMessageEvent.man \
	XChangePointerControl.man \
	XChangeSaveSet.man \
	XChangeWindowAttributes.man \
	XCreateWindowEvent.man \
	XCirculateEvent.man \
	XCirculateRequestEvent.man \
	XClearArea.man \
	XColormapEvent.man \
	XConfigureEvent.man \
	XConfigureRequestEvent.man \
	XConfigureWindow.man \
	XCopyArea.man \
	XCreateColormap.man \
	XCreateFontCursor.man \
	XCreateFontSet.man \
	XCreateGC.man \
	XCreateIC.man \
	XInitImage.man \
	XCreateOC.man \
	XCreatePixmap.man \
	XCreateRegion.man \
	XCreateWindow.man \
	XCrossingEvent.man \
	XDestroyWindowEvent.man \
	XDefineCursor.man \
	XDestroyWindow.man \
	XDrawArc.man \
	XDrawImageString.man \
	XDrawLine.man \
	XDrawPoint.man \
	XDrawRectangle.man \
	XDrawString.man \
	XDrawText.man \
	XEmptyRegion.man \
	XrmEnumerateDatabase.man \
	XExtentsOfFontSet.man \
	XErrorEvent.man \
	XExposeEvent.man \
	XFocusChangeEvent.man \
	XFilterEvent.man \
	XFontSetExtents.man \
	XFillRectangle.man \
	XFlush.man \
	XFontsOfFontSet.man \
	XFree.man \
	XGraphicsExposeEvent.man \
	XGravityEvent.man \
	XrmGetFileDatabase.man \
	XrmGetResource.man \
	XGetVisualInfo.man \
	XGetWindowAttributes.man \
	XGetWindowProperty.man \
	XGetXCBConnection.man \
	XGrabButton.man \
	XGrabKey.man \
	XGrabKeyboard.man \
	XGrabPointer.man \
	XGrabServer.man \
	XIconifyWindow.man \
	XIfEvent.man \
	XrmInitialize.man \
	XInstallColormap.man \
	XAddConnectionWatch.man \
	XIntersectRegion.man \
	XInternAtom.man \
	XKeymapEvent.man \
	XListFonts.man \
	XLoadFont.man \
	XLookupKeysym.man \
	XrmMergeDatabases.man \
	XMapEvent.man \
	XMapRequestEvent.man \
	XMapWindow.man \
	XNextEvent.man \
	XNoOp.man \
	XOpenDisplay.man \
	XOpenIM.man \
	XOpenOM.man \
	XPropertyEvent.man \
	XParseGeometry.man \
	XPolygonRegion.man \
	XPutBackEvent.man \
	XPutImage.man \
	XrmPutResource.man \
	XQueryBestSize.man \
	XQueryColor.man \
	XQueryExtension.man \
	XQueryPointer.man \
	XQueryTree.man \
	XReparentEvent.man \
	XResourceManagerString.man \
	XResizeRequestEvent.man \
	XRaiseWindow.man \
	XReadBitmapFile.man \
	XRecolorCursor.man \
	XReparentWindow.man \
	XSelectionClearEvent.man \
	XSaveContext.man \
	XSelectionEvent.man \
	XSetICFocus.man \
	XSetICValues.man \
	XSelectInput.man \
	XStringListToTextProperty.man \
	XSelectionRequestEvent.man \
	XSetArcMode.man \
	XSetClipOrigin.man \
	XSetCloseDownMode.man \
	XSetCommand.man \
	XSetErrorHandler.man \
	XSetEventQueueOwner.man \
	XSendEvent.man \
	XSetFillStyle.man \
	XSetFont.man \
	XSetFontPath.man \
	XSetInputFocus.man \
	XSetLineAttributes.man \
	XSetPointerMapping.man \
	XSetScreenSaver.man \
	XSetSelectionOwner.man \
	XSetState.man \
	XSetTransientForHint.man \
	XSetTextProperty.man \
	XSetTile.man \
	XSetWMClientMachine.man \
	XSetWMColormapWindows.man \
	XSetWMIconName.man \
	XSetWMName.man \
	XSetWMProperties.man \
	XSetWMProtocols.man \
	XStoreBytes.man \
	XStoreColors.man \
	XStringToKeysym.man \
	XSupportsLocale.man \
	XSynchronize.man \
	XmbTextListToTextProperty.man \
	XTextExtents.man \
	XTextWidth.man \
	XInitThreads.man \
	XTranslateCoordinates.man \
	XrmUniqueQuark.man \
	XUnmapEvent.man \
	XUnmapWindow.man \
	XVaCreateNestedList.man \
	XVisibilityEvent.man \
	XWarpPointer.man \
	XcmsCCCOfColormap.man \
	XcmsAllocColor.man \
	XcmsConvertColors.man \
	XcmsColor.man \
	XcmsCreateCCC.man \
	XcmsDefaultCCC.man \
	XcmsCIELabQueryMaxC.man \
	XcmsCIELuvQueryMaxC.man \
	XcmsQueryBlack.man \
	XcmsQueryColor.man \
	XcmsStoreColor.man \
	XcmsSetWhitePoint.man \
	XcmsTekHVCQueryMaxC.man \
	XmbDrawImageString.man \
	XmbDrawString.man \
	XmbDrawText.man \
	XmbLookupString.man \
	XmbResetIC.man \
	XmbTextEscapement.man \
	XmbTextExtents.man \
	XmbTextPerCharExtents.man

# What we actually install 

if MANPAGES
libman_DATA = $(libman_PRE:man=@LIB_MAN_SUFFIX@) \
	      $(all_shadows:=.@LIB_MAN_SUFFIX@)

BUILT_SOURCES = shadows.DONE
endif

EXTRA_DIST = $(libman_PRE)

CLEANFILES = $(libman_DATA)

include $(top_srcdir)/cpprules.in

SUFFIXES += .$(LIB_MAN_SUFFIX) .man

XORGRELSTRING = @PACKAGE_STRING@
  XORGMANNAME = X Version 11

MANDEFS = -D__libmansuffix__=$(LIB_MAN_SUFFIX) \
	-D__xorgversion__="\"$(XORGRELSTRING)\" \"$(XORGMANNAME)\""

CPP_MAN_FLAGS = $(MANDEFS) $(EXTRAMANDEFS)

.man.$(LIB_MAN_SUFFIX):
	$(RAWCPP) $(RAWCPPFLAGS) $(CPP_MAN_FLAGS) < $< | $(CPP_SED_MAGIC) > $@

# Generate man page shadow files (Replaces InstallManPageAliases from Imake)
all_shadows =                                \
    $(AllPlanes_shadows)                     \
    $(BlackPixelOfScreen_shadows)            \
    $(DisplayOfCCC_shadows)                  \
    $(ImageByteOrder_shadows)                \
    $(IsCursorKey_shadows)                   \
    $(XAllocClassHint_shadows)               \
    $(XAllocIconSize_shadows)                \
    $(XAllocStandardColormap_shadows)        \
    $(XAllocSizeHints_shadows)               \
    $(XAllocWMHints_shadows)                 \
    $(XAddHost_shadows)                      \
    $(XAllocColor_shadows)                   \
    $(XAnyEvent_shadows)                     \
    $(XButtonEvent_shadows)                  \
    $(XChangeKeyboardControl_shadows)        \
    $(XChangeKeyboardMapping_shadows)        \
    $(XChangePointerControl_shadows)         \
    $(XChangeSaveSet_shadows)                \
    $(XChangeWindowAttributes_shadows)       \
    $(XClearArea_shadows)                    \
    $(XConfigureWindow_shadows)              \
    $(XCopyArea_shadows)                     \
    $(XCreateColormap_shadows)               \
    $(XCreateFontCursor_shadows)             \
    $(XCreateFontSet_shadows)                \
    $(XCreateGC_shadows)                     \
    $(XCreateIC_shadows)                     \
    $(XInitImage_shadows)                    \
    $(XCreateOC_shadows)                     \
    $(XCreatePixmap_shadows)                 \
    $(XCreateRegion_shadows)                 \
    $(XCreateWindow_shadows)                 \
    $(XDefineCursor_shadows)                 \
    $(XDestroyWindow_shadows)                \
    $(XDrawArc_shadows)                      \
    $(XDrawImageString_shadows)              \
    $(XDrawLine_shadows)                     \
    $(XDrawPoint_shadows)                    \
    $(XDrawRectangle_shadows)                \
    $(XDrawString_shadows)                   \
    $(XDrawText_shadows)                     \
    $(XEmptyRegion_shadows)                  \
    $(XFillRectangle_shadows)                \
    $(XFlush_shadows)                        \
    $(XFontsOfFontSet_shadows)               \
    $(XGraphicsExposeEvent_shadows)          \
    $(XrmGetFileDatabase_shadows)            \
    $(XrmGetResource_shadows)                \
    $(XGetVisualInfo_shadows)                \
    $(XGetWindowAttributes_shadows)          \
    $(XGetWindowProperty_shadows)            \
    $(XGrabButton_shadows)                   \
    $(XGrabKey_shadows)                      \
    $(XGrabKeyboard_shadows)                 \
    $(XGrabPointer_shadows)                  \
    $(XGrabServer_shadows)                   \
    $(XIconifyWindow_shadows)                \
    $(XIfEvent_shadows)                      \
    $(XrmInitialize_shadows)                 \
    $(XInstallColormap_shadows)              \
    $(XAddConnectionWatch_shadows)           \
    $(XIntersectRegion_shadows)              \
    $(XInternAtom_shadows)                   \
    $(XListFonts_shadows)                    \
    $(XLoadFont_shadows)                     \
    $(XLookupKeysym_shadows)                 \
    $(XrmMergeDatabases_shadows)             \
    $(XMapEvent_shadows)                     \
    $(XMapWindow_shadows)                    \
    $(XNextEvent_shadows)                    \
    $(XOpenDisplay_shadows)                  \
    $(XOpenIM_shadows)                       \
    $(XOpenOM_shadows)                       \
    $(XParseGeometry_shadows)                \
    $(XPolygonRegion_shadows)                \
    $(XPutImage_shadows)                     \
    $(XrmPutResource_shadows)                \
    $(XQueryBestSize_shadows)                \
    $(XQueryColor_shadows)                   \
    $(XQueryExtension_shadows)               \
    $(XResourceManagerString_shadows)        \
    $(XRaiseWindow_shadows)                  \
    $(XReadBitmapFile_shadows)               \
    $(XRecolorCursor_shadows)                \
    $(XSaveContext_shadows)                  \
    $(XSetICFocus_shadows)                   \
    $(XSetICValues_shadows)                  \
    $(XStringListToTextProperty_shadows)     \
    $(XSetArcMode_shadows)                   \
    $(XSetClipOrigin_shadows)                \
    $(XSetCloseDownMode_shadows)             \
    $(XSetCommand_shadows)                   \
    $(XSetErrorHandler_shadows)              \
    $(XSendEvent_shadows)                    \
    $(XSetFillStyle_shadows)                 \
    $(XSetFontPath_shadows)                  \
    $(XSetInputFocus_shadows)                \
    $(XSetLineAttributes_shadows)            \
    $(XSetPointerMapping_shadows)            \
    $(XSetScreenSaver_shadows)               \
    $(XSetSelectionOwner_shadows)            \
    $(XSetState_shadows)                     \
    $(XSetTransientForHint_shadows)          \
    $(XSetTextProperty_shadows)              \
    $(XSetTile_shadows)                      \
    $(XSetWMClientMachine_shadows)           \
    $(XSetWMColormapWindows_shadows)         \
    $(XSetWMIconName_shadows)                \
    $(XSetWMName_shadows)                    \
    $(XSetWMProperties_shadows)              \
    $(XSetWMProtocols_shadows)               \
    $(XStoreBytes_shadows)                   \
    $(XStoreColors_shadows)                  \
    $(XStringToKeysym_shadows)               \
    $(XSupportsLocale_shadows)               \
    $(XSynchronize_shadows)                  \
    $(XmbTextListToTextProperty_shadows)     \
    $(XTextExtents_shadows)                  \
    $(XTextWidth_shadows)                    \
    $(XInitThreads_shadows)                  \
    $(XrmUniqueQuark_shadows)                \
    $(XUnmapWindow_shadows)                  \
    $(XcmsCCCOfColormap_shadows)             \
    $(XcmsAllocColor_shadows)                \
    $(XcmsColor_shadows)                     \
    $(XcmsCreateCCC_shadows)                 \
    $(XcmsCIELabQueryMaxC_shadows)           \
    $(XcmsCIELuvQueryMaxC_shadows)           \
    $(XcmsQueryBlack_shadows)                \
    $(XcmsQueryColor_shadows)                \
    $(XcmsStoreColor_shadows)                \
    $(XcmsSetWhitePoint_shadows)             \
    $(XcmsTekHVCQueryMaxC_shadows)           \
    $(XmbDrawImageString_shadows)            \
    $(XmbDrawString_shadows)                 \
    $(XmbDrawText_shadows)                   \
    $(XmbLookupString_shadows)               \
    $(XmbResetIC_shadows)                    \
    $(XmbTextEscapement_shadows)             \
    $(XmbTextExtents_shadows)                \
    $(XmbTextPerCharExtents_shadows)


AllPlanes_shadows =                         \
    BlackPixel                              \
    WhitePixel                              \
    ConnectionNumber                        \
    DefaultColormap                         \
    DefaultDepth                            \
    XListDepths                             \
    DefaultGC                               \
    DefaultRootWindow                       \
    DefaultScreenOfDisplay                  \
    DefaultScreen                           \
    DefaultVisual                           \
    DisplayCells                            \
    DisplayPlanes                           \
    DisplayString                           \
    XMaxRequestSize                         \
    XExtendedMaxRequestSize                 \
    LastKnownRequestProcessed               \
    NextRequest                             \
    ProtocolVersion                         \
    ProtocolRevision                        \
    QLength                                 \
    RootWindow                              \
    ScreenCount                             \
    ScreenOfDisplay                         \
    ServerVendor                            \
    VendorRelease

BlackPixelOfScreen_shadows =                \
    WhitePixelOfScreen                      \
    CellsOfScreen                           \
    DefaultColormapOfScreen                 \
    DefaultDepthOfScreen                    \
    DefaultGCOfScreen                       \
    DefaultVisualOfScreen                   \
    DoesBackingStore                        \
    DoesSaveUnders                          \
    DisplayOfScreen                         \
    XScreenNumberOfScreen                   \
    EventMaskOfScreen                       \
    HeightOfScreen                          \
    HeightMMOfScreen                        \
    MaxCmapsOfScreen                        \
    MinCmapsOfScreen                        \
    PlanesOfScreen                          \
    RootWindowOfScreen                      \
    WidthOfScreen                           \
    WidthMMOfScreen

DisplayOfCCC_shadows =                      \
    VisualOfCCC                             \
    ScreenNumberOfCCC                       \
    ScreenWhitePointOfCCC                   \
    ClientWhitePointOfCCC

ImageByteOrder_shadows =                    \
    BitmapBitOrder                          \
    BitmapPad                               \
    BitmapUnit                              \
    DisplayHeight                           \
    DisplayHeightMM                         \
    DisplayWidth                            \
    DisplayWidthMM                          \
    XListPixmapFormats                      \
    XPixmapFormatValues

IsCursorKey_shadows =                       \
    IsFunctionKey                           \
    IsKeypadKey                             \
    IsMiscFunctionKey                       \
    IsModifierKey                           \
    IsPFKey                                 \
    IsPrivateKeypadKey

XAllocClassHint_shadows =                   \
    XSetClassHint                           \
    XGetClassHint                           \
    XClassHint

XAllocIconSize_shadows =                    \
    XSetIconSizes                           \
    XGetIconSizes                           \
    XIconSize

XAllocStandardColormap_shadows =            \
    XSetRGBColormaps                        \
    XGetRGBColormaps                        \
    XStandardColormap

XAllocSizeHints_shadows =                   \
    XSetWMNormalHints                       \
    XGetWMNormalHints                       \
    XSetWMSizeHints                         \
    XGetWMSizeHints                         \
    XSizeHints

XAllocWMHints_shadows =                     \
    XSetWMHints                             \
    XGetWMHints                             \
    XWMHints

XAddHost_shadows =                          \
    XAddHosts                               \
    XListHosts                              \
    XRemoveHost                             \
    XRemoveHosts                            \
    XSetAccessControl                       \
    XEnableAccessControl                    \
    XDisableAccessControl                   \
    XHostAddress

XAllocColor_shadows =                       \
    XAllocNamedColor                        \
    XAllocColorCells                        \
    XAllocColorPlanes                       \
    XFreeColors

XAnyEvent_shadows =                         \
    XEvent

XButtonEvent_shadows =                      \
    XKeyEvent                               \
    XMotionEvent

XChangeKeyboardControl_shadows =            \
    XGetKeyboardControl                     \
    XAutoRepeatOn                           \
    XAutoRepeatOff                          \
    XBell                                   \
    XQueryKeymap                            \
    XKeyboardControl

XChangeKeyboardMapping_shadows =            \
    XGetKeyboardMapping                     \
    XDisplayKeycodes                        \
    XSetModifierMapping                     \
    XGetModifierMapping                     \
    XNewModifiermap                         \
    XInsertModifiermapEntry                 \
    XDeleteModifiermapEntry                 \
    XFreeModifierMap                        \
    XModifierKeymap

XChangePointerControl_shadows =             \
    XGetPointerControl

XChangeSaveSet_shadows =                    \
    XAddToSaveSet                           \
    XRemoveFromSaveSet

XChangeWindowAttributes_shadows =           \
    XSetWindowBackground                    \
    XSetWindowBackgroundPixmap              \
    XSetWindowBorder                        \
    XSetWindowBorderPixmap                  \
    XSetWindowColormap

XClearArea_shadows =                        \
    XClearWindow

XConfigureWindow_shadows =                  \
    XMoveWindow                             \
    XResizeWindow                           \
    XMoveResizeWindow                       \
    XSetWindowBorderWidth                   \
    XWindowChanges

XCopyArea_shadows =                         \
    XCopyPlane

XCreateColormap_shadows =                   \
    XCopyColormapAndFree                    \
    XFreeColormap                           \
    XColor

XCreateFontCursor_shadows =                 \
    XCreatePixmapCursor                     \
    XCreateGlyphCursor

XCreateFontSet_shadows =                    \
    XFreeFontSet

XCreateGC_shadows =                         \
    XCopyGC                                 \
    XChangeGC                               \
    XGetGCValues                            \
    XFreeGC                                 \
    XGContextFromGC                         \
    XGCValues

XCreateIC_shadows =                         \
    XDestroyIC                              \
    XIMOfIC

XInitImage_shadows =                        \
    XCreateImage                            \
    XGetPixel                               \
    XPutPixel                               \
    XSubImage                               \
    XAddPixel                               \
    XDestroyImage

XCreateOC_shadows =                         \
    XDestroyOC                              \
    XSetOCValues                            \
    XGetOCValues                            \
    XOMOfOC

XCreatePixmap_shadows =                     \
    XFreePixmap

XCreateRegion_shadows =                     \
    XSetRegion                              \
    XDestroyRegion

XCreateWindow_shadows =                     \
    XCreateSimpleWindow                     \
    XSetWindowAttributes

XDefineCursor_shadows =                     \
    XUndefineCursor

XDestroyWindow_shadows =                    \
    XDestroySubwindows

XDrawArc_shadows =                          \
    XDrawArcs                               \
    XArc

XDrawImageString_shadows =                  \
    XDrawImageString16

XDrawLine_shadows =                         \
    XDrawLines                              \
    XDrawSegments                           \
    XSegment

XDrawPoint_shadows =                        \
    XDrawPoints                             \
    XPoint

XDrawRectangle_shadows =                    \
    XDrawRectangles                         \
    XRectangle

XDrawString_shadows =                       \
    XDrawString16

XDrawText_shadows =                         \
    XDrawText16                             \
    XTextItem                               \
    XTextItem16

XEmptyRegion_shadows =                      \
    XEqualRegion                            \
    XPointInRegion                          \
    XRectInRegion

XFillRectangle_shadows =                    \
    XFillRectangles                         \
    XFillPolygon                            \
    XFillArc                                \
    XFillArcs

XFlush_shadows =                            \
    XSync                                   \
    XEventsQueued                           \
    XPending

XFontsOfFontSet_shadows =                   \
    XBaseFontNameListOfFontSet              \
    XLocaleOfFontSet                        \
    XContextDependentDrawing                \
    XContextualDrawing                      \
    XDirectionalDependentDrawing

XGraphicsExposeEvent_shadows =              \
    XNoExposeEvent

XrmGetFileDatabase_shadows =                \
    XrmPutFileDatabase                      \
    XrmGetStringDatabase                    \
    XrmLocaleOfDatabase                     \
    XrmGetDatabase                          \
    XrmSetDatabase                          \
    XrmDestroyDatabase

XrmGetResource_shadows =                    \
    XrmQGetResource                         \
    XrmQGetSearchList                       \
    XrmQGetSearchResource

XGetVisualInfo_shadows =                    \
    XMatchVisualInfo                        \
    XVisualIDFromVisual                     \
    XVisualInfo

XGetWindowAttributes_shadows =              \
    XGetGeometry                            \
    XWindowAttributes

XGetWindowProperty_shadows =                \
    XListProperties                         \
    XChangeProperty                         \
    XRotateWindowProperties                 \
    XDeleteProperty

XGrabButton_shadows =                       \
    XUngrabButton

XGrabKey_shadows =                          \
    XUngrabKey

XGrabKeyboard_shadows =                     \
    XUngrabKeyboard

XGrabPointer_shadows =                      \
    XUngrabPointer                          \
    XChangeActivePointerGrab

XGrabServer_shadows =                       \
    XUngrabServer

XIconifyWindow_shadows =                    \
    XWithdrawWindow                         \
    XReconfigureWMWindow

XIfEvent_shadows =                          \
    XCheckIfEvent                           \
    XPeekIfEvent

XrmInitialize_shadows =                     \
    XrmParseCommand                         \
    XrmValue                                \
    XrmOptionKind                           \
    XrmOptionDescRec

XInstallColormap_shadows =                  \
    XUninstallColormap                      \
    XListInstalledColormaps

XAddConnectionWatch_shadows =               \
    XRemoveConnectionWatch                  \
    XProcessInternalConnection              \
    XInternalConnectionNumbers

XIntersectRegion_shadows =                  \
    XUnionRegion                            \
    XUnionRectWithRegion                    \
    XSubtractRegion                         \
    XXorRegion                              \
    XOffsetRegion                           \
    XShrinkRegion

XInternAtom_shadows =                       \
    XInternAtoms                            \
    XGetAtomName                            \
    XGetAtomNames

XListFonts_shadows =                        \
    XFreeFontNames                          \
    XListFontsWithInfo                      \
    XFreeFontInfo

XLoadFont_shadows =                         \
    XQueryFont                              \
    XLoadQueryFont                          \
    XFreeFont                               \
    XGetFontProperty                        \
    XUnloadFont                             \
    XCharStruct                             \
    XFontProp                               \
    XChar2b                                 \
    XFontStruct

XLookupKeysym_shadows =                     \
    XRefreshKeyboardMapping                 \
    XLookupString                           \
    XRebindKeysym

XrmMergeDatabases_shadows =                 \
    XrmCombineDatabase                      \
    XrmCombineFileDatabase

XMapEvent_shadows =                         \
    XMappingEvent

XMapWindow_shadows =                        \
    XMapRaised                              \
    XMapSubwindows

XNextEvent_shadows =                        \
    XPeekEvent                              \
    XWindowEvent                            \
    XCheckWindowEvent                       \
    XMaskEvent                              \
    XCheckMaskEvent                         \
    XCheckTypedEvent                        \
    XCheckTypedWindowEvent

XOpenDisplay_shadows =                      \
    XCloseDisplay

XOpenIM_shadows =                           \
    XCloseIM                                \
    XSetIMValues                            \
    XGetIMValues                            \
    XDisplayOfIM                            \
    XLocaleOfIM                             \
    XRegisterIMInstantiateCallback          \
    XUnregisterIMInstantiateCallback

XOpenOM_shadows =                           \
    XCloseOM                                \
    XSetOMValues                            \
    XGetOMValues                            \
    XDisplayOfOM                            \
    XLocaleOfOM

XParseGeometry_shadows =                    \
    XWMGeometry

XPolygonRegion_shadows =                    \
    XClipBox

XPutImage_shadows =                         \
    XGetImage                               \
    XGetSubImage

XrmPutResource_shadows =                    \
    XrmQPutResource                         \
    XrmPutStringResource                    \
    XrmQPutStringResource                   \
    XrmPutLineResource

XQueryBestSize_shadows =                    \
    XQueryBestTile                          \
    XQueryBestStipple

XQueryColor_shadows =                       \
    XQueryColors                            \
    XLookupColor                            \
    XParseColor

XQueryExtension_shadows =                   \
    XListExtensions                         \
    XFreeExtensionList

XResourceManagerString_shadows =            \
    XScreenResourceString

XRaiseWindow_shadows =                      \
    XLowerWindow                            \
    XCirculateSubwindows                    \
    XCirculateSubwindowsUp                  \
    XCirculateSubwindowsDown                \
    XRestackWindows

XReadBitmapFile_shadows =                   \
    XReadBitmapFileData                     \
    XWriteBitmapFile                        \
    XCreatePixmapFromBitmapData             \
    XCreateBitmapFromData

XRecolorCursor_shadows =                    \
    XFreeCursor                             \
    XQueryBestCursor

XSaveContext_shadows =                      \
    XFindContext                            \
    XDeleteContext                          \
    XUniqueContext

XSetICFocus_shadows =                       \
    XUnsetICFocus

XSetICValues_shadows =                      \
    XGetICValues

XStringListToTextProperty_shadows =         \
    XTextPropertyToStringList               \
    XFreeStringList                         \
    XTextProperty

XSetArcMode_shadows =                       \
    XSetSubwindowMode                       \
    XSetGraphicsExposure

XSetClipOrigin_shadows =                    \
    XSetClipMask                            \
    XSetClipRectangles

XSetCloseDownMode_shadows =                 \
    XKillClient

XSetCommand_shadows =                       \
    XGetCommand

XSetErrorHandler_shadows =                  \
    XGetErrorText                           \
    XDisplayName                            \
    XSetIOErrorHandler                      \
    XGetErrorDatabaseText

XSendEvent_shadows =                        \
    XDisplayMotionBufferSize                \
    XGetMotionEvents                        \
    XTimeCoord

XSetFillStyle_shadows =                     \
    XSetFillRule

XSetFontPath_shadows =                      \
    XGetFontPath                            \
    XFreeFontPath

XSetInputFocus_shadows =                    \
    XGetInputFocus

XSetLineAttributes_shadows =                \
    XSetDashes

XSetPointerMapping_shadows =                \
    XGetPointerMapping

XSetScreenSaver_shadows =                   \
    XForceScreenSaver                       \
    XActivateScreenSaver                    \
    XResetScreenSaver                       \
    XGetScreenSaver

XSetSelectionOwner_shadows =                \
    XGetSelectionOwner                      \
    XConvertSelection

XSetState_shadows =                         \
    XSetFunction                            \
    XSetPlaneMask                           \
    XSetForeground                          \
    XSetBackground

XSetTransientForHint_shadows =              \
    XGetTransientForHint

XSetTextProperty_shadows =                  \
    XGetTextProperty

XSetTile_shadows =                          \
    XSetStipple                             \
    XSetTSOrigin

XSetWMClientMachine_shadows =               \
    XGetWMClientMachine

XSetWMColormapWindows_shadows =             \
    XGetWMColormapWindows

XSetWMIconName_shadows =                    \
    XGetWMIconName                          \
    XSetIconName                            \
    XGetIconName

XSetWMName_shadows =                        \
    XGetWMName                              \
    XStoreName                              \
    XFetchName

XSetWMProperties_shadows =                  \
    XmbSetWMProperties                      \
    Xutf8SetWMProperties

XSetWMProtocols_shadows =                   \
    XGetWMProtocols

XStoreBytes_shadows =                       \
    XStoreBuffer                            \
    XFetchBytes                             \
    XFetchBuffer                            \
    XRotateBuffers

XStoreColors_shadows =                      \
    XStoreColor                             \
    XStoreNamedColor

XStringToKeysym_shadows =                   \
    XKeysymToString                         \
    XKeycodeToKeysym                        \
    XKeysymToKeycode                        \
    XConvertCase

XSupportsLocale_shadows =                   \
    XSetLocaleModifiers

XSynchronize_shadows =                      \
    XSetAfterFunction

XmbTextListToTextProperty_shadows =         \
    XwcTextListToTextProperty               \
    Xutf8TextListToTextProperty             \
    XmbTextPropertyToTextList               \
    XwcTextPropertyToTextList               \
    Xutf8TextPropertyToTextList             \
    XwcFreeStringList                       \
    XDefaultString

XTextExtents_shadows =                      \
    XTextExtents16                          \
    XQueryTextExtents                       \
    XQueryTextExtents16

XTextWidth_shadows =                        \
    XTextWidth16

XInitThreads_shadows =                      \
    XLockDisplay                            \
    XUnlockDisplay

XrmUniqueQuark_shadows =                    \
    XrmStringToQuark                        \
    XrmPermStringToQuark                    \
    XrmQuarkToString                        \
    XrmStringToQuarkList                    \
    XrmStringToBindingQuarkList

XUnmapWindow_shadows =                      \
    XUnmapSubwindows

XcmsCCCOfColormap_shadows =                 \
    XcmsSetCCCOfColormap

XcmsAllocColor_shadows =                    \
    XcmsAllocNamedColor

XcmsColor_shadows =                         \
    XcmsRGB                                 \
    XcmsRGBi                                \
    XcmsCIEXYZ                              \
    XcmsCIEuvY                              \
    XcmsCIExyY                              \
    XcmsCIELab                              \
    XcmsCIELuv                              \
    XcmsTekHVC                              \
    XcmsPad

XcmsCreateCCC_shadows =                     \
    XcmsFreeCCC

XcmsCIELabQueryMaxC_shadows =               \
    XcmsCIELabQueryMaxL                     \
    XcmsCIELabQueryMaxLC                    \
    XcmsCIELabQueryMinL

XcmsCIELuvQueryMaxC_shadows =               \
    XcmsCIELuvQueryMaxL                     \
    XcmsCIELuvQueryMaxLC                    \
    XcmsCIELuvQueryMinL

XcmsQueryBlack_shadows =                    \
    XcmsQueryBlue                           \
    XcmsQueryGreen                          \
    XcmsQueryRed                            \
    XcmsQueryWhite

XcmsQueryColor_shadows =                    \
    XcmsQueryColors                         \
    XcmsLookupColor

XcmsStoreColor_shadows =                    \
    XcmsStoreColors

XcmsSetWhitePoint_shadows =                 \
    XcmsSetWhiteAdjustProc

XcmsTekHVCQueryMaxC_shadows =               \
    XcmsTekHVCQueryMaxV                     \
    XcmsTekHVCQueryMaxVC                    \
    XcmsTekHVCQueryMaxVSamples              \
    XcmsTekHVCQueryMinV

XmbDrawImageString_shadows =                \
    XwcDrawImageString                      \
    Xutf8DrawImageString

XmbDrawString_shadows =                     \
    XwcDrawString                           \
    Xutf8DrawString

XmbDrawText_shadows =                       \
    XwcDrawText                             \
    Xutf8DrawText

XmbLookupString_shadows =                   \
    XwcLookupString                         \
    Xutf8LookupString

XmbResetIC_shadows =                        \
    XwcResetIC                              \
    Xutf8ResetIC

XmbTextEscapement_shadows =                 \
    XwcTextEscapement                       \
    Xutf8TextEscapement

XmbTextExtents_shadows =                    \
    XwcTextExtents                          \
    Xutf8TextExtents

XmbTextPerCharExtents_shadows =             \
    XwcTextPerCharExtents                   \
    Xutf8TextPerCharExtents

shadows.DONE:
	-rm -f $(all_aliases:=.@LIB_MAN_SUFFIX@)
	(for i in $(AllPlanes_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/AllPlanes.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(BlackPixelOfScreen_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/BlackPixelOfScreen.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(DisplayOfCCC_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/DisplayOfCCC.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(ImageByteOrder_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/ImageByteOrder.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(IsCursorKey_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/IsCursorKey.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XAllocClassHint_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XAllocClassHint.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XAllocIconSize_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XAllocIconSize.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XAllocStandardColormap_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XAllocStandardColormap.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XAllocSizeHints_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XAllocSizeHints.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XAllocWMHints_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XAllocWMHints.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XAddHost_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XAddHost.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XAllocColor_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XAllocColor.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XAnyEvent_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XAnyEvent.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XButtonEvent_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XButtonEvent.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XChangeKeyboardControl_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XChangeKeyboardControl.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XChangeKeyboardMapping_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XChangeKeyboardMapping.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XChangePointerControl_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XChangePointerControl.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XChangeSaveSet_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XChangeSaveSet.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XChangeWindowAttributes_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XChangeWindowAttributes.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XClearArea_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XClearArea.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XConfigureWindow_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XConfigureWindow.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XCopyArea_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XCopyArea.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XCreateColormap_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XCreateColormap.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XCreateFontCursor_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XCreateFontCursor.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XCreateFontSet_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XCreateFontSet.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XCreateGC_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XCreateGC.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XCreateIC_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XCreateIC.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XInitImage_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XInitImage.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XCreateOC_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XCreateOC.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XCreatePixmap_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XCreatePixmap.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XCreateRegion_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XCreateRegion.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XCreateWindow_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XCreateWindow.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XDefineCursor_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XDefineCursor.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XDestroyWindow_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XDestroyWindow.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XDrawArc_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XDrawArc.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XDrawImageString_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XDrawImageString.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XDrawLine_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XDrawLine.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XDrawPoint_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XDrawPoint.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XDrawRectangle_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XDrawRectangle.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XDrawString_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XDrawString.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XDrawText_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XDrawText.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XEmptyRegion_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XEmptyRegion.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XFillRectangle_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XFillRectangle.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XFlush_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XFlush.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XFontsOfFontSet_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XFontsOfFontSet.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XGraphicsExposeEvent_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XGraphicsExposeEvent.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XrmGetFileDatabase_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XrmGetFileDatabase.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XrmGetResource_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XrmGetResource.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XGetVisualInfo_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XGetVisualInfo.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XGetWindowAttributes_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XGetWindowAttributes.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XGetWindowProperty_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XGetWindowProperty.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XGrabButton_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XGrabButton.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XGrabKey_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XGrabKey.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XGrabKeyboard_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XGrabKeyboard.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XGrabPointer_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XGrabPointer.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XGrabServer_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XGrabServer.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XIconifyWindow_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XIconifyWindow.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XIfEvent_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XIfEvent.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XrmInitialize_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XrmInitialize.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XInstallColormap_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XInstallColormap.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XAddConnectionWatch_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XAddConnectionWatch.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XIntersectRegion_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XIntersectRegion.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XInternAtom_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XInternAtom.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XListFonts_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XListFonts.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XLoadFont_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XLoadFont.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XLookupKeysym_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XLookupKeysym.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XrmMergeDatabases_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XrmMergeDatabases.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XMapEvent_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XMapEvent.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XMapWindow_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XMapWindow.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XNextEvent_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XNextEvent.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XOpenDisplay_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XOpenDisplay.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XOpenIM_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XOpenIM.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XOpenOM_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XOpenOM.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XParseGeometry_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XParseGeometry.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XPolygonRegion_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XPolygonRegion.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XPutImage_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XPutImage.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XrmPutResource_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XrmPutResource.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XQueryBestSize_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XQueryBestSize.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XQueryColor_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XQueryColor.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XQueryExtension_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XQueryExtension.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XResourceManagerString_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XResourceManagerString.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XRaiseWindow_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XRaiseWindow.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XReadBitmapFile_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XReadBitmapFile.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XRecolorCursor_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XRecolorCursor.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSaveContext_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSaveContext.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetICFocus_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetICFocus.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetICValues_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetICValues.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XStringListToTextProperty_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XStringListToTextProperty.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetArcMode_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetArcMode.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetClipOrigin_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetClipOrigin.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetCloseDownMode_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetCloseDownMode.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetCommand_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetCommand.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetErrorHandler_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetErrorHandler.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSendEvent_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSendEvent.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetFillStyle_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetFillStyle.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetFontPath_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetFontPath.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetInputFocus_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetInputFocus.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetLineAttributes_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetLineAttributes.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetPointerMapping_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetPointerMapping.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetScreenSaver_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetScreenSaver.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetSelectionOwner_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetSelectionOwner.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetState_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetState.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetTransientForHint_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetTransientForHint.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetTextProperty_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetTextProperty.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetTile_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetTile.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetWMClientMachine_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetWMClientMachine.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetWMColormapWindows_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetWMColormapWindows.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetWMIconName_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetWMIconName.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetWMName_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetWMName.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetWMProperties_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetWMProperties.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSetWMProtocols_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSetWMProtocols.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XStoreBytes_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XStoreBytes.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XStoreColors_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XStoreColors.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XStringToKeysym_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XStringToKeysym.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSupportsLocale_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSupportsLocale.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XSynchronize_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XSynchronize.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XmbTextListToTextProperty_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XmbTextListToTextProperty.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XTextExtents_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XTextExtents.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XTextWidth_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XTextWidth.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XInitThreads_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XInitThreads.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XrmUniqueQuark_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XrmUniqueQuark.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XUnmapWindow_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XUnmapWindow.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XcmsCCCOfColormap_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XcmsCCCOfColormap.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XcmsAllocColor_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XcmsAllocColor.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XcmsColor_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XcmsColor.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XcmsCreateCCC_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XcmsCreateCCC.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XcmsCIELabQueryMaxC_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XcmsCIELabQueryMaxC.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XcmsCIELuvQueryMaxC_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XcmsCIELuvQueryMaxC.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XcmsQueryBlack_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XcmsQueryBlack.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XcmsQueryColor_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XcmsQueryColor.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XcmsStoreColor_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XcmsStoreColor.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XcmsSetWhitePoint_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XcmsSetWhitePoint.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XcmsTekHVCQueryMaxC_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XcmsTekHVCQueryMaxC.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XmbDrawImageString_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XmbDrawImageString.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XmbDrawString_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XmbDrawString.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XmbDrawText_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XmbDrawText.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XmbLookupString_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XmbLookupString.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XmbResetIC_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XmbResetIC.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XmbTextEscapement_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XmbTextEscapement.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XmbTextExtents_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XmbTextExtents.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
	(for i in $(XmbTextPerCharExtents_shadows:=.@LIB_MAN_SUFFIX@) ; do \
	 echo .so man$(LIB_MAN_DIR_SUFFIX)/XmbTextPerCharExtents.$(LIB_MAN_SUFFIX) > $$i; \
	 done)
