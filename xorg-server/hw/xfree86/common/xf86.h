/*
 * Copyright (c) 1997-2003 by The XFree86 Project, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
 */

/*
 * This file contains declarations for public XFree86 functions and variables,
 * and definitions of public macros.
 *
 * "public" means available to video drivers.
 */

#ifndef _XF86_H
#define _XF86_H

#if HAVE_XORG_CONFIG_H
#include <xorg-config.h>
#elif HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "xf86str.h"
#include "xf86Opt.h"
#include <X11/Xfuncproto.h>
#include <stdarg.h>
#ifdef RANDR
#include <X11/extensions/randr.h>
#endif

#include "propertyst.h"

/* General parameters */
extern _X_EXPORT int xf86DoConfigure;
extern _X_EXPORT int xf86DoShowOptions;
extern _X_EXPORT Bool xf86DoConfigurePass1;

extern _X_EXPORT DevPrivateKeyRec xf86ScreenKeyRec;

#define xf86ScreenKey (&xf86ScreenKeyRec)

extern _X_EXPORT DevPrivateKeyRec xf86CreateRootWindowKeyRec;

#define xf86CreateRootWindowKey (&xf86CreateRootWindowKeyRec)

extern _X_EXPORT ScrnInfoPtr *xf86Screens;      /* List of pointers to ScrnInfoRecs */
extern _X_EXPORT const unsigned char byte_reversed[256];
extern _X_EXPORT Bool fbSlotClaimed;

#if (defined(__sparc__) || defined(__sparc)) && !defined(__OpenBSD__)
extern _X_EXPORT Bool sbusSlotClaimed;
#endif

#if defined(XSERVER_PLATFORM_BUS)
extern _X_EXPORT int platformSlotClaimed;
#endif

extern _X_EXPORT confDRIRec xf86ConfigDRI;
extern _X_EXPORT Bool xf86DRI2Enabled(void);

extern _X_EXPORT Bool VTSwitchEnabled;  /* kbd driver */

#define XF86SCRNINFO(p) xf86ScreenToScrn(p)

#define XF86FLIP_PIXELS() \
	do { \
	    if (xf86GetFlipPixels()) { \
		pScreen->whitePixel = (pScreen->whitePixel) ? 0 : 1; \
		pScreen->blackPixel = (pScreen->blackPixel) ? 0 : 1; \
	   } \
	while (0)

#define BOOLTOSTRING(b) ((b) ? "TRUE" : "FALSE")

#define PIX24TOBPP(p) (((p) == Pix24Use24) ? 24 : \
			(((p) == Pix24Use32) ? 32 : 0))

/* Function Prototypes */
#ifndef _NO_XF86_PROTOTYPES

/* PCI related */
#ifdef XSERVER_LIBPCIACCESS
#include <pciaccess.h>
extern _X_EXPORT int pciSlotClaimed;

extern _X_EXPORT Bool xf86CheckPciSlot(const struct pci_device *);
extern _X_EXPORT int xf86ClaimPciSlot(struct pci_device *, DriverPtr drvp,
                                      int chipset, GDevPtr dev, Bool active);
extern _X_EXPORT void xf86UnclaimPciSlot(struct pci_device *, GDevPtr dev);
extern _X_EXPORT Bool xf86ParsePciBusString(const char *busID, int *bus,
                                            int *device, int *func);
extern _X_EXPORT Bool xf86ComparePciBusString(const char *busID, int bus,
                                              int device, int func);
extern _X_EXPORT Bool xf86IsPrimaryPci(struct pci_device *pPci);
extern _X_EXPORT Bool xf86CheckPciMemBase(struct pci_device *pPci,
                                          memType base);
extern _X_EXPORT struct pci_device *xf86GetPciInfoForEntity(int entityIndex);
extern _X_EXPORT int xf86MatchPciInstances(const char *driverName,
                                           int vendorID, SymTabPtr chipsets,
                                           PciChipsets * PCIchipsets,
                                           GDevPtr * devList, int numDevs,
                                           DriverPtr drvp, int **foundEntities);
extern _X_EXPORT ScrnInfoPtr xf86ConfigPciEntity(ScrnInfoPtr pScrn,
                                                 int scrnFlag, int entityIndex,
                                                 PciChipsets * p_chip,
                                                 void *dummy, EntityProc init,
                                                 EntityProc enter,
                                                 EntityProc leave,
                                                 pointer private);
/* Obsolete! don't use */
extern _X_EXPORT Bool xf86ConfigActivePciEntity(ScrnInfoPtr pScrn,
                                                int entityIndex,
                                                PciChipsets * p_chip,
                                                void *dummy, EntityProc init,
                                                EntityProc enter,
                                                EntityProc leave,
                                                pointer private);
#else
#define xf86VGAarbiterInit() do {} while (0)
#define xf86VGAarbiterFini() do {} while (0)
#define xf86VGAarbiterLock(x) do {} while (0)
#define xf86VGAarbiterUnlock(x) do {} while (0)
#define xf86VGAarbiterScrnInit(x) do {} while (0)
#define xf86VGAarbiterDeviceDecodes() do {} while (0)
#define xf86VGAarbiterWrapFunctions() do {} while (0)
#endif

/* xf86Bus.c */

extern _X_EXPORT int xf86GetFbInfoForScreen(int scrnIndex);
extern _X_EXPORT int xf86ClaimFbSlot(DriverPtr drvp, int chipset, GDevPtr dev,
                                     Bool active);
extern _X_EXPORT int xf86ClaimNoSlot(DriverPtr drvp, int chipset, GDevPtr dev,
                                     Bool active);
extern _X_EXPORT Bool xf86DriverHasEntities(DriverPtr drvp);
extern _X_EXPORT void xf86AddEntityToScreen(ScrnInfoPtr pScrn, int entityIndex);
extern _X_EXPORT void xf86SetEntityInstanceForScreen(ScrnInfoPtr pScrn,
                                                     int entityIndex,
                                                     int instance);
extern _X_EXPORT int xf86GetNumEntityInstances(int entityIndex);
extern _X_EXPORT GDevPtr xf86GetDevFromEntity(int entityIndex, int instance);
extern _X_EXPORT void xf86RemoveEntityFromScreen(ScrnInfoPtr pScrn,
                                                 int entityIndex);
extern _X_EXPORT EntityInfoPtr xf86GetEntityInfo(int entityIndex);
extern _X_EXPORT Bool xf86SetEntityFuncs(int entityIndex, EntityProc init,
                                         EntityProc enter, EntityProc leave,
                                         pointer);
extern _X_EXPORT Bool xf86IsEntityPrimary(int entityIndex);
extern _X_EXPORT ScrnInfoPtr xf86FindScreenForEntity(int entityIndex);

extern _X_EXPORT int xf86GetLastScrnFlag(int entityIndex);
extern _X_EXPORT void xf86SetLastScrnFlag(int entityIndex, int scrnIndex);
extern _X_EXPORT Bool xf86IsEntityShared(int entityIndex);
extern _X_EXPORT void xf86SetEntityShared(int entityIndex);
extern _X_EXPORT Bool xf86IsEntitySharable(int entityIndex);
extern _X_EXPORT void xf86SetEntitySharable(int entityIndex);
extern _X_EXPORT Bool xf86IsPrimInitDone(int entityIndex);
extern _X_EXPORT void xf86SetPrimInitDone(int entityIndex);
extern _X_EXPORT void xf86ClearPrimInitDone(int entityIndex);
extern _X_EXPORT int xf86AllocateEntityPrivateIndex(void);
extern _X_EXPORT DevUnion *xf86GetEntityPrivate(int entityIndex, int privIndex);

/* xf86Configure.c */
extern _X_EXPORT GDevPtr xf86AddBusDeviceToConfigure(const char *driver,
                                                     BusType bus, void *busData,
                                                     int chipset);

/* xf86Cursor.c */

extern _X_EXPORT void xf86LockZoom(ScreenPtr pScreen, int lock);
extern _X_EXPORT void xf86InitViewport(ScrnInfoPtr pScr);
extern _X_EXPORT void xf86SetViewport(ScreenPtr pScreen, int x, int y);
extern _X_EXPORT void xf86ZoomViewport(ScreenPtr pScreen, int zoom);
extern _X_EXPORT Bool xf86SwitchMode(ScreenPtr pScreen, DisplayModePtr mode);
extern _X_EXPORT void *xf86GetPointerScreenFuncs(void);
extern _X_EXPORT void xf86InitOrigins(void);
extern _X_EXPORT void xf86ReconfigureLayout(void);

/* xf86cvt.c */
extern _X_EXPORT DisplayModePtr xf86CVTMode(int HDisplay, int VDisplay,
                                            float VRefresh, Bool Reduced,
                                            Bool Interlaced);

/* xf86DPMS.c */

extern _X_EXPORT Bool xf86DPMSInit(ScreenPtr pScreen, DPMSSetProcPtr set,
                                   int flags);

#ifdef DPMSExtension
extern _X_EXPORT int DPMSSet(ClientPtr client, int level);
extern _X_EXPORT Bool DPMSSupported(void);
#endif

/* xf86DGA.c */

#ifdef XFreeXDGA
extern _X_EXPORT Bool DGAInit(ScreenPtr pScreen, DGAFunctionPtr funcs,
                              DGAModePtr modes, int num);
extern _X_EXPORT Bool DGAReInitModes(ScreenPtr pScreen, DGAModePtr modes,
                                     int num);
extern _X_EXPORT xf86SetDGAModeProc xf86SetDGAMode;
#endif

/* xf86Events.c */

extern _X_EXPORT void SetTimeSinceLastInputEvent(void);
extern _X_EXPORT pointer xf86AddInputHandler(int fd, InputHandlerProc proc,
                                             pointer data);
extern _X_EXPORT int xf86RemoveInputHandler(pointer handler);
extern _X_EXPORT void xf86DisableInputHandler(pointer handler);
extern _X_EXPORT void xf86EnableInputHandler(pointer handler);
extern _X_EXPORT pointer xf86AddGeneralHandler(int fd, InputHandlerProc proc,
                                               pointer data);
extern _X_EXPORT int xf86RemoveGeneralHandler(pointer handler);
extern _X_EXPORT void xf86DisableGeneralHandler(pointer handler);
extern _X_EXPORT void xf86EnableGeneralHandler(pointer handler);
extern _X_EXPORT InputHandlerProc xf86SetConsoleHandler(InputHandlerProc
                                                        handler, pointer data);
extern _X_EXPORT void xf86InterceptSignals(int *signo);
extern _X_EXPORT void xf86InterceptSigIll(void (*sigillhandler) (void));
extern _X_EXPORT Bool xf86EnableVTSwitch(Bool new);
extern _X_EXPORT void xf86ProcessActionEvent(ActionEvent action, void *arg);
extern _X_EXPORT void xf86PrintBacktrace(void);

/* xf86Helper.c */

extern _X_EXPORT void xf86AddDriver(DriverPtr driver, pointer module,
                                    int flags);
extern _X_EXPORT void xf86DeleteDriver(int drvIndex);
extern _X_EXPORT ScrnInfoPtr xf86AllocateScreen(DriverPtr drv, int flags);
extern _X_EXPORT void xf86DeleteScreen(ScrnInfoPtr pScrn);
extern _X_EXPORT int xf86AllocateScrnInfoPrivateIndex(void);
extern _X_EXPORT Bool xf86AddPixFormat(ScrnInfoPtr pScrn, int depth, int bpp,
                                       int pad);
extern _X_EXPORT Bool xf86SetDepthBpp(ScrnInfoPtr scrp, int depth, int bpp,
                                      int fbbpp, int depth24flags);
extern _X_EXPORT void xf86PrintDepthBpp(ScrnInfoPtr scrp);
extern _X_EXPORT Bool xf86SetWeight(ScrnInfoPtr scrp, rgb weight, rgb mask);
extern _X_EXPORT Bool xf86SetDefaultVisual(ScrnInfoPtr scrp, int visual);
extern _X_EXPORT Bool xf86SetGamma(ScrnInfoPtr scrp, Gamma newGamma);
extern _X_EXPORT void xf86SetDpi(ScrnInfoPtr pScrn, int x, int y);
extern _X_EXPORT void xf86SetBlackWhitePixels(ScreenPtr pScreen);
extern _X_EXPORT void xf86EnableDisableFBAccess(ScrnInfoPtr pScrn, Bool enable);
extern _X_EXPORT void
xf86VDrvMsgVerb(int scrnIndex, MessageType type, int verb,
                const char *format, va_list args)
_X_ATTRIBUTE_PRINTF(4, 0);
extern _X_EXPORT void
xf86DrvMsgVerb(int scrnIndex, MessageType type, int verb,
               const char *format, ...)
_X_ATTRIBUTE_PRINTF(4, 5);
extern _X_EXPORT void
xf86DrvMsg(int scrnIndex, MessageType type, const char *format, ...)
_X_ATTRIBUTE_PRINTF(3, 4);
extern _X_EXPORT void
xf86MsgVerb(MessageType type, int verb, const char *format, ...)
_X_ATTRIBUTE_PRINTF(3, 4);
extern _X_EXPORT void
xf86Msg(MessageType type, const char *format, ...)
_X_ATTRIBUTE_PRINTF(2, 3);
extern _X_EXPORT void
xf86ErrorFVerb(int verb, const char *format, ...)
_X_ATTRIBUTE_PRINTF(2, 3);
extern _X_EXPORT void
xf86ErrorF(const char *format, ...)
_X_ATTRIBUTE_PRINTF(1, 2);
extern _X_EXPORT const char *
xf86TokenToString(SymTabPtr table, int token);
extern _X_EXPORT int
xf86StringToToken(SymTabPtr table, const char *string);
extern _X_EXPORT void
xf86ShowClocks(ScrnInfoPtr scrp, MessageType from);
extern _X_EXPORT void
xf86PrintChipsets(const char *drvname, const char *drvmsg, SymTabPtr chips);
extern _X_EXPORT int
xf86MatchDevice(const char *drivername, GDevPtr ** driversectlist);
extern _X_EXPORT const char *
xf86GetVisualName(int visual);
extern _X_EXPORT int
xf86GetVerbosity(void);
extern _X_EXPORT Pix24Flags
xf86GetPix24(void);
extern _X_EXPORT int
xf86GetDepth(void);
extern _X_EXPORT rgb
xf86GetWeight(void);
extern _X_EXPORT Gamma
xf86GetGamma(void);
extern _X_EXPORT Bool
xf86GetFlipPixels(void);
extern _X_EXPORT const char *
xf86GetServerName(void);
extern _X_EXPORT Bool
xf86ServerIsExiting(void);
extern _X_EXPORT Bool
xf86ServerIsResetting(void);
extern _X_EXPORT Bool
xf86ServerIsInitialising(void);
extern _X_EXPORT Bool
xf86ServerIsOnlyDetecting(void);
extern _X_EXPORT Bool
xf86CaughtSignal(void);
extern _X_EXPORT Bool
xf86GetVidModeAllowNonLocal(void);
extern _X_EXPORT Bool
xf86GetVidModeEnabled(void);
extern _X_EXPORT Bool
xf86GetModInDevAllowNonLocal(void);
extern _X_EXPORT Bool
xf86GetModInDevEnabled(void);
extern _X_EXPORT Bool
xf86GetAllowMouseOpenFail(void);
extern _X_EXPORT void
xf86DisableRandR(void);
extern _X_EXPORT CARD32
xorgGetVersion(void);
extern _X_EXPORT CARD32
xf86GetModuleVersion(pointer module);
extern _X_EXPORT pointer
xf86LoadDrvSubModule(DriverPtr drv, const char *name);
extern _X_EXPORT pointer
xf86LoadSubModule(ScrnInfoPtr pScrn, const char *name);
extern _X_EXPORT pointer
xf86LoadOneModule(char *name, pointer optlist);
extern _X_EXPORT void
xf86UnloadSubModule(pointer mod);
extern _X_EXPORT Bool
xf86LoaderCheckSymbol(const char *name);
extern _X_EXPORT void
xf86SetBackingStore(ScreenPtr pScreen);
extern _X_EXPORT void
xf86SetSilkenMouse(ScreenPtr pScreen);
extern _X_EXPORT pointer
xf86FindXvOptions(ScrnInfoPtr pScrn, int adapt_index, char *port_name,
                  char **adaptor_name, pointer *adaptor_options);
extern _X_EXPORT void
xf86GetOS(const char **name, int *major, int *minor, int *teeny);
extern _X_EXPORT ScrnInfoPtr
xf86ConfigFbEntity(ScrnInfoPtr pScrn, int scrnFlag,
                   int entityIndex, EntityProc init,
                   EntityProc enter, EntityProc leave, pointer private);

extern _X_EXPORT Bool
xf86IsScreenPrimary(ScrnInfoPtr pScrn);
extern _X_EXPORT int
xf86RegisterRootWindowProperty(int ScrnIndex, Atom property, Atom type,
                               int format, unsigned long len, pointer value);
extern _X_EXPORT Bool
xf86IsUnblank(int mode);

/* xf86Init.c */

extern _X_EXPORT PixmapFormatPtr
xf86GetPixFormat(ScrnInfoPtr pScrn, int depth);
extern _X_EXPORT int
xf86GetBppFromDepth(ScrnInfoPtr pScrn, int depth);

/* xf86Mode.c */

extern _X_EXPORT int
xf86GetNearestClock(ScrnInfoPtr scrp, int freq, Bool allowDiv2,
                    int DivFactor, int MulFactor, int *divider);
extern _X_EXPORT const char *
xf86ModeStatusToString(ModeStatus status);
extern _X_EXPORT ModeStatus
xf86LookupMode(ScrnInfoPtr scrp, DisplayModePtr modep,
               ClockRangePtr clockRanges, LookupModeFlags strategy);
extern _X_EXPORT ModeStatus
xf86CheckModeForMonitor(DisplayModePtr mode, MonPtr monitor);
extern _X_EXPORT ModeStatus
xf86InitialCheckModeForDriver(ScrnInfoPtr scrp, DisplayModePtr mode,
                              ClockRangePtr clockRanges,
                              LookupModeFlags strategy,
                              int maxPitch, int virtualX, int virtualY);
extern _X_EXPORT ModeStatus
xf86CheckModeForDriver(ScrnInfoPtr scrp, DisplayModePtr mode, int flags);
extern _X_EXPORT int
xf86ValidateModes(ScrnInfoPtr scrp, DisplayModePtr availModes,
                  char **modeNames, ClockRangePtr clockRanges,
                  int *linePitches, int minPitch, int maxPitch,
                  int minHeight, int maxHeight, int pitchInc,
                  int virtualX, int virtualY, int apertureSize,
                  LookupModeFlags strategy);
extern _X_EXPORT void
xf86DeleteMode(DisplayModePtr * modeList, DisplayModePtr mode);
extern _X_EXPORT void
xf86PruneDriverModes(ScrnInfoPtr scrp);
extern _X_EXPORT void
xf86SetCrtcForModes(ScrnInfoPtr scrp, int adjustFlags);
extern _X_EXPORT void
xf86PrintModes(ScrnInfoPtr scrp);
extern _X_EXPORT void
xf86ShowClockRanges(ScrnInfoPtr scrp, ClockRangePtr clockRanges);
extern _X_EXPORT double
xf86ModeHSync(const DisplayModeRec * mode);
extern _X_EXPORT double
xf86ModeVRefresh(const DisplayModeRec * mode);
extern _X_EXPORT void
xf86SetModeDefaultName(DisplayModePtr mode);
extern _X_EXPORT void
xf86SetModeCrtc(DisplayModePtr p, int adjustFlags);
extern _X_EXPORT DisplayModePtr
xf86DuplicateMode(const DisplayModeRec * pMode);
extern _X_EXPORT void
xf86SaveModeContents(DisplayModePtr intern, const DisplayModeRec * pMode);
extern _X_EXPORT DisplayModePtr
xf86DuplicateModes(ScrnInfoPtr pScrn, DisplayModePtr modeList);
extern _X_EXPORT Bool
xf86ModesEqual(const DisplayModeRec * pMode1, const DisplayModeRec * pMode2);
extern _X_EXPORT void
xf86PrintModeline(int scrnIndex, DisplayModePtr mode);
extern _X_EXPORT DisplayModePtr
xf86ModesAdd(DisplayModePtr modes, DisplayModePtr new);

/* xf86Option.c */

extern _X_EXPORT void
xf86CollectOptions(ScrnInfoPtr pScrn, XF86OptionPtr extraOpts);

/* xf86RandR.c */
#ifdef RANDR
extern _X_EXPORT Bool
xf86RandRInit(ScreenPtr pScreen);
extern _X_EXPORT Rotation
xf86GetRotation(ScreenPtr pScreen);
extern _X_EXPORT Bool
xf86RandRSetNewVirtualAndDimensions(ScreenPtr pScreen,
                                    int newvirtX, int newvirtY,
                                    int newmmWidth, int newmmHeight,
                                    Bool resetMode);
#endif

/* xf86Extensions.c */
extern void xf86ExtensionInit(void);

/* convert ScreenPtr to ScrnInfoPtr */
extern _X_EXPORT ScrnInfoPtr xf86ScreenToScrn(ScreenPtr pScreen);
/* convert ScrnInfoPtr to ScreenPtr */
extern _X_EXPORT ScreenPtr xf86ScrnToScreen(ScrnInfoPtr pScrn);

#endif                          /* _NO_XF86_PROTOTYPES */

#define XF86_HAS_SCRN_CONV 1 /* define for drivers to use in api compat */

#define XF86_SCRN_INTERFACE 1 /* define for drivers to use in api compat */

/* flags passed to xf86 allocate screen */
#define XF86_ALLOCATE_GPU_SCREEN 1

#endif                          /* _XF86_H */
